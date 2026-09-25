//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Real WMSH vertex, material and baked-light inputs through the
//          scene-derived native Vulkan PBR world shader.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_surface_host.h"
#include "../../materialsystem/shaderapivulkan/vulkan_device.h"
#include "../../materialsystem/shaderapivulkan/vulkan_world_lightmap.h"
#include "../mapcontainertest/world_lightmap_cases.h"
#include "mapcontainer/world_lightmap.h"
#include "render/light_set.h"
#include "render/pbr_brdf.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace
{

struct WorldVertex
{
	float position[3];
	std::int16_t normal[2];
	std::int16_t tangent[2];
	std::int8_t handedness;
	std::uint8_t padding[3];
	float uv[2];
	float lightmapUv[2];
};
static_assert( sizeof( WorldVertex ) == 40, "WMSH vertex layout changed" );

bool DrawWorld( render_vulkan::CVulkanContext &context, std::uint8_t *red, std::string *error )
{
	context.ClearDynamicQueue();
	context.SetClearColor( 0, 0, 0, 1 );
	if ( !context.QueueWorldMeshBatch( 0, 3 ) )
	{
		*error = "WMSH PBR draw was not accepted";
		return false;
	}
	context.RequestCapture();
	bool skip = false;
	if ( !context.BeginFrame( &skip, error ) || skip || !context.EndFrame( error ) )
		return false;
	int width = 0;
	int height = 0;
	const std::vector<std::uint8_t> &pixels = context.GetCapturedPixels( &width, &height );
	if ( width < 4 || height < 4 || pixels.size() < size_t( width * height * 4 ) )
	{
		*error = "WMSH PBR capture is incomplete";
		return false;
	}
	*red = pixels[( size_t( height / 2 ) * width + width / 2 ) * 4];
	return true;
}

bool UploadLmap( render_vulkan::CVulkanContext &context, const std::vector<char> &bytes,
    std::uint32_t version, std::string *error )
{
	mapcontainer::WorldLightmapLayout layout{};
	const mapcontainer::WorldLightmapError validation =
	    mapcontainer::ValidateWorldLightmap( bytes.data(), bytes.size(), version, &layout );
	if ( validation != mapcontainer::WorldLightmapError::Ok )
	{
		*error = mapcontainer::WorldLightmapErrorName( validation );
		return false;
	}
	world_mesh_gpu::WorldLightmapUploadRequest request;
	request.width = layout.width;
	request.height = layout.height;
	request.layerCount = layout.layerCount;
	for ( std::uint32_t i = 0; i < layout.layerCount; ++i )
	{
		request.layers[i] = bytes.data() + layout.layerOffset[i];
		request.roles[i] = static_cast<world_mesh_gpu::WorldLightmapRole>( layout.roles[i] );
	}
	return render_vulkan::UploadWorldLightmapLayers( context, request, error );
}

} // namespace

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SKIP: SDL video unavailable: %s\n", SDL_GetError() );
		return 77;
	}
	SDL_Window *window = SDL_CreateWindow( "wmsh-pbr-pixels", 128, 128, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "SKIP: Vulkan window unavailable: %s\n", SDL_GetError() );
		SDL_Quit();
		return 77;
	}
	std::string error;
	std::unique_ptr<render_vulkan::IVulkanSurfaceHost> host =
	    render_vulkan::MakeSdl3LegacySurfaceHost( window, &error );
	render_vulkan::CVulkanContext context;
	render_vulkan::VulkanContextConfig config;
	config.appName = "wmsh-pbr-pixels";
	config.enableValidation = true;
	if ( !host || !context.Init( *host, config, &error ) )
	{
		std::fprintf( stderr, "SKIP: native Vulkan context unavailable: %s\n", error.c_str() );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 77;
	}
	int checks = 0;
	int failures = 0;
	const auto check = [&]( bool condition, const char *message )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			std::fprintf( stderr, "FAIL: %s (%s)\n", message, error.c_str() );
		}
	};
	check( context.InitDynamicMesh( &error ) && context.PbrWorldPipelineSupported(),
	    "scene-derived WMSH PBR pipeline initializes" );
	const std::array<WorldVertex, 3> vertices = { {
	    { { -0.75f, -0.75f, 0.5f }, { 0, 0 }, { 32767, 0 }, 1, { 0, 0, 0 }, { 0.5f, 0.5f },
	        { 0.5f, 0.5f } },
	    { { 0.75f, -0.75f, 0.5f }, { 0, 0 }, { 32767, 0 }, 1, { 0, 0, 0 }, { 0.5f, 0.5f },
	        { 0.5f, 0.5f } },
	    { { 0.0f, 0.75f, 0.5f }, { 0, 0 }, { 32767, 0 }, 1, { 0, 0, 0 }, { 0.5f, 0.5f },
	        { 0.5f, 0.5f } },
	} };
	const std::array<std::uint32_t, 3> indices = { 0, 1, 2 };
	check( context.UploadWorldMesh(
	           vertices.data(), sizeof( vertices ), indices.data(), sizeof( indices ), &error ),
	    "packed normal and tangent vertices upload without rebuilding" );
	const int base = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_SRGB, &error );
	const int mrao = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	const int normal = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8_UNORM, &error );
	bool imagesReady = base >= 0 && mrao >= 0 && normal >= 0;
	check( imagesReady, "PBR base, mask, normal and HDR lightmap images are supported" );
	if ( imagesReady )
	{
		const std::array<std::uint8_t, 4> baseBytes = { 200, 0, 0, 255 };
		const std::array<std::uint8_t, 4> dielectric = { 0, 128, 255, 255 };
		const std::array<std::uint8_t, 4> metal = { 255, 128, 255, 255 };
		const std::array<std::uint8_t, 4> roughMetal = { 255, 255, 255, 255 };
		const std::array<std::uint8_t, 2> normalUp = { 128, 128 };
		const std::array<std::uint8_t, 2> normalTilt = { 255, 128 };
		check( context.UploadManagedTexture( base, baseBytes.data(), baseBytes.size(), &error ) &&
		           context.UploadManagedTexture(
		               mrao, dielectric.data(), dielectric.size(), &error ) &&
		           context.UploadManagedTexture( normal, normalUp.data(), normalUp.size(), &error ),
		    "PBR material texels upload" );
		// LMAP v1: the checked-in single-page KTX2, through the map
		// container's validator and the layered upload contract.
		std::ifstream package( "quality/fixtures/ktx2/white-1x1-rgba16f.ktx2", std::ios::binary );
		std::vector<char> packageBytes(
		    std::istreambuf_iterator<char>{ package }, std::istreambuf_iterator<char>{} );
		check( !packageBytes.empty(), "linear HDR lightmap KTX2 fixture exists" );
		if ( !packageBytes.empty() )
		{
			auto damaged = packageBytes;
			damaged[0] = 0;
			check( mapcontainer::ValidateWorldLightmap( damaged.data(), damaged.size(), 1 ) ==
			           mapcontainer::WorldLightmapError::BadIdentifier,
			    "corrupt BSP2 lightmap package is rejected" );
			check( mapcontainer::ValidateWorldLightmap( packageBytes.data(), packageBytes.size(),
			           2 ) == mapcontainer::WorldLightmapError::VersionMismatch,
			    "a single page is not accepted as a layered (v2) lump" );
			error.clear();
			check( UploadLmap( context, packageBytes, 1, &error ),
			    "BSP2 lightmap KTX2 uploads as the map-scoped HDR image" );
			check( context.WorldLightmapIndirectHandle() < 0, "a v1 page has no indirect layer" );
		}
		context.BindManagedTexture( base );
		context.SelectDynamicColorSpace( render_vulkan::CVulkanContext::kColorSrgbReadBase |
		                                 render_vulkan::CVulkanContext::kColorSrgbWrite );
		const float eye[3] = { 0.0f, 0.0f, 1.0f };
		check( !context.SelectPbrWorldMaterial( base, normal, eye, -1.0f ),
		    "WMSH material selection rejects an sRGB MRAO input" );
		check( context.SelectPbrWorldMaterial( mrao, normal, eye, -1.0f ),
		    "WMSH material selection binds validated PBR images" );
		render_vulkan::CVulkanContext::PbrWorldScene scene;
		scene.lightRadiance[0] = scene.lightRadiance[1] = scene.lightRadiance[2] = 0.0f;
		context.SetDynamicPbrWorldScene( scene );
		std::uint8_t diffuse = 0;
		check( DrawWorld( context, &diffuse, &error ), "dielectric world pixel renders" );
		// Cycles' color-free diffuse bake is the white Lambertian response,
		// rather than incident irradiance. A second 1/pi would darken this
		// base-200 texel below the expected sRGB range.
		check( diffuse >= 175 && diffuse <= 210,
		    "Cycles diffuse bake is multiplied by albedo without another 1/pi" );
		// RFC 0011 indirect-light debug view. A v1 page has no indirect
		// layer: the view is black rather than showing the total light.
		{
			const auto srgbByte = []( float linear )
			{
				const float encoded = linear <= 0.0031308f
				                          ? linear * 12.92f
				                          : 1.055f * std::pow( linear, 1.0f / 2.4f ) - 0.055f;
				return encoded * 255.0f;
			};
			std::uint8_t viewed = 0;
			context.SetIndirectLightView( 1, 1.0f );
			check( DrawWorld( context, &viewed, &error ) && viewed <= 1,
			    "indirect view of a map without an indirect layer is black" );
			// LMAP v2: total 1.0, indirect 0.25 (red channel; halves 0x3400).
			const std::vector<std::vector<std::uint16_t>> layers = {
			    { 0x3c00, 0x3c00, 0x3c00, 0x3c00 }, { 0x3400, 0x3800, 0x3000, 0x3c00 } };
			const std::vector<char> layered = lmap_cases::MakeLmap( 1, 1, 2, layers );
			check( mapcontainer::ValidateWorldLightmap( layered.data(), layered.size(), 1 ) ==
			           mapcontainer::WorldLightmapError::VersionMismatch,
			    "a layered page is not accepted as a v1 lump" );
			std::vector<char> truncated( layered.begin(), layered.end() - 8 );
			check( mapcontainer::ValidateWorldLightmap( truncated.data(), truncated.size(), 2 ) ==
			           mapcontainer::WorldLightmapError::InvalidLevelIndex,
			    "a truncated layered page is rejected" );
			check( UploadLmap( context, layered, 2, &error ) &&
			           context.WorldLightmapIndirectHandle() >= 0,
			    "LMAP v2 total and indirect layers upload" );
			check( DrawWorld( context, &viewed, &error ), "indirect light view renders" );
			std::fprintf(
			    stderr, "indirect light view red %u (expected %.1f)\n", viewed, srgbByte( 0.25f ) );
			check( std::abs( viewed - srgbByte( 0.25f ) ) <= 1.5f,
			    "view 1 writes the indirect layer's diffuse light, without albedo" );
			// View 2: times the linear base (sRGB 200) with metal 0, occlusion 1.
			const float baseLinear = std::pow( ( 200.0f / 255.0f + 0.055f ) / 1.055f, 2.4f );
			context.SetIndirectLightView( 2, 1.0f );
			check( DrawWorld( context, &viewed, &error ) &&
			           std::abs( viewed - srgbByte( 0.25f * baseLinear ) ) <= 1.5f,
			    "view 2 writes the indirect diffuse radiance" );
			// Seeded defect: an indirect view doubled by its scale is detected.
			context.SetIndirectLightView( 1, 2.0f );
			check( DrawWorld( context, &viewed, &error ) &&
			           std::abs( viewed - srgbByte( 0.25f ) ) > 20.0f,
			    "a doubled indirect view differs from the indirect layer" );
			context.SetIndirectLightView( 0, 1.0f );
			std::uint8_t restored = 0;
			check( DrawWorld( context, &restored, &error ) && restored == diffuse,
			    "turning the view off restores the full shading of the total layer" );

			// RFC 0011 G2: the light set's unbaked lights as direct light, over a
			// black bake so the pixel is theirs alone. The centre pixel is world
			// (0, 0, 0.5), normal +Z, seen from the eye at (0, 0, 1).
			check( context.DirectLightsSupported(), "WMSH PBR adds direct lights on this device" );
			const std::vector<char> black =
			    lmap_cases::MakeLmap( 1, 1, 2, { { 0, 0, 0, 0x3c00 }, { 0, 0, 0, 0x3c00 } } );
			check( UploadLmap( context, black, 2, &error ), "a black lightmap uploads" );
			std::uint8_t dark = 255;
			check( DrawWorld( context, &dark, &error ) && dark <= 1,
			    "without lights a black bake is black" );
			render_vulkan::CVulkanContext::DirectLight point;
			point.position[0] = 0.2f;
			point.position[2] = 0.8f;
			point.radius = 1.0f;
			point.minLight = 0.05f;
			point.color[0] = point.color[1] = point.color[2] = 1.0f;
			// The CPU model: pi x incident x the layered BRDF x N.L, incident
			// the light set's legacy falloff; `cosine` and `falloff` off are the
			// seeded controls.
			const auto expectedLinear = [&](
			                                const render_vulkan::CVulkanContext::DirectLight &light,
			                                bool cosine, bool falloff )
			{
				const float toLight[3] = {
				    light.position[0], light.position[1], light.position[2] - 0.5f };
				const float distanceSquared =
				    toLight[0] * toLight[0] + toLight[1] * toLight[1] + toLight[2] * toLight[2];
				const float distance = std::sqrt( distanceSquared );
				const float l[3] = {
				    toLight[0] / distance, toLight[1] / distance, toLight[2] / distance };
				const float h[3] = { l[0], l[1], l[2] + 1.0f };
				const float hLength = std::sqrt( h[0] * h[0] + h[1] * h[1] + h[2] * h[2] );
				const float normalDotLight = l[2];
				const float normalDotHalf = h[2] / hLength;
				const float viewDotHalf = h[2] / hLength;
				const float baseRed = std::pow( ( 200.0f / 255.0f + 0.055f ) / 1.055f, 2.4f );
				const render::pbr::Color brdf =
				    render::pbr::EvaluateLayeredDirect( { baseRed, 0.0f, 0.0f }, 0.0f, 1.0f,
				        normalDotLight, normalDotHalf, viewDotHalf, 128.0f / 255.0f );
				const float incident =
				    light.color[0] *
				    ( falloff ? light_set::Falloff( distanceSquared, light.radius, light.minLight )
				              : 1.0f );
				return render::pbr::kPi * incident * brdf.red * ( cosine ? normalDotLight : 1.0f );
			};
			context.SetDirectLights( &point, 1 );
			std::uint8_t lit = 0;
			check( DrawWorld( context, &lit, &error ), "a direct point light renders" );
			const float expected = srgbByte( expectedLinear( point, true, true ) );
			std::fprintf( stderr, "direct point light red %u (expected %.1f)\n", lit, expected );
			check( std::abs( lit - expected ) <= 3.0f,
			    "a point light adds the layered BRDF through the legacy falloff and N.L" );
			check( std::abs( lit - srgbByte( expectedLinear( point, false, true ) ) ) > 6.0f &&
			           std::abs( lit - srgbByte( expectedLinear( point, true, false ) ) ) > 6.0f,
			    "the model's cosine and falloff terms are both visible (seeded controls)" );
			render_vulkan::CVulkanContext::DirectLight far = point;
			far.position[2] = 2.0f; // 1.5 away, beyond its radius of 1
			render_vulkan::CVulkanContext::DirectLight away = point;
			away.direction[2] = 1.0f; // a spot aimed away from the surface
			away.outerCos = 0.9f;
			away.innerCos = 0.95f;
			const render_vulkan::CVulkanContext::DirectLight unlitPair[2] = { far, away };
			context.SetDirectLights( unlitPair, 2 );
			std::uint8_t unlit = 255;
			check( DrawWorld( context, &unlit, &error ) && unlit <= 1,
			    "a light beyond its radius and a spot aimed away add nothing" );
			render_vulkan::CVulkanContext::DirectLight aimed = away;
			aimed.direction[0] = -0.2f; // at the surface point: fully inside its cone
			aimed.direction[2] = -0.3f;
			const render_vulkan::CVulkanContext::DirectLight pair[2] = { point, aimed };
			context.SetDirectLights( pair, 2 );
			std::uint8_t both = 0;
			check( DrawWorld( context, &both, &error ) &&
			           std::abs( both - srgbByte( 2.0f * expectedLinear( point, true, true ) ) ) <=
			               3.0f,
			    "two lights add, and a spot aimed at the surface lights it like a point" );
			context.SetDirectLights( nullptr, 0 );

			// render.indirect-policy.v1 on the furnace's values (total 0.75,
			// direct 0.3, indirect 0.45): RuntimeIndirect (the direct layer plus
			// the baked producer's indirect) matches Baked (the total layer);
			// the seeded double count (the total layer under RuntimeIndirect)
			// does not.
			const std::vector<char> furnace = lmap_cases::MakeLmap( 1, 1, 3,
			    { { 0x3a00, 0x3a00, 0x3a00, 0x3c00 }, { 0x34cd, 0x34cd, 0x34cd, 0x3c00 },
			        { 0x3733, 0x3733, 0x3733, 0x3c00 } } );
			check( UploadLmap( context, furnace, 2, &error ) &&
			           context.WorldLightmapDirectHandle() >= 0,
			    "a furnace-valued LMAP v2 with direct and indirect layers uploads" );
			std::uint8_t baked = 0, runtime = 0, seeded = 0;
			context.SetIndirectPolicy( 0, false );
			check( DrawWorld( context, &baked, &error ), "Baked policy renders" );
			context.SetIndirectPolicy( 2, false );
			check( context.EffectiveIndirectPolicy() == 2 && DrawWorld( context, &runtime, &error ),
			    "RuntimeIndirect policy renders" );
			context.SetIndirectPolicy( 2, true );
			check( DrawWorld( context, &seeded, &error ), "the seeded double count renders" );
			std::fprintf( stderr, "furnace policy red: baked %u runtime %u seeded double %u\n",
			    baked, runtime, seeded );
			check( std::abs( int( baked ) - int( runtime ) ) <= 1,
			    "RuntimeIndirect composes the furnace's total light as Baked does" );
			check( seeded > baked + 10, "the seeded double count (total + indirect) is detected" );
			context.SetIndirectPolicy( 0, false );
			check( UploadLmap( context, layered, 2, &error ) &&
			           ( context.SetIndirectPolicy( 2, false ),
			               context.EffectiveIndirectPolicy() ) == 0,
			    "RuntimeIndirect is unavailable without a direct layer" );
			context.SetIndirectPolicy( 0, false );
			check( UploadLmap( context, layered, 2, &error ), "the layered lightmap is restored" );
			check( DrawWorld( context, &restored, &error ) && restored == diffuse,
			    "without direct lights the shading is the bake's again" );
		}
		render_vulkan::CVulkanContext::DynRasterState raster;
		raster.cullMode = VK_CULL_MODE_BACK_BIT;
		context.SelectDynamicRasterState( raster );
		std::uint8_t front = 0;
		check( DrawWorld( context, &front, &error ), "front-facing WMSH PBR pixel renders" );
		check( front > 20, "WMSH PBR uses the world mesh front-face winding" );
		raster.cullMode = VK_CULL_MODE_NONE;
		context.SelectDynamicRasterState( raster );
		check( context.UploadManagedTexture( mrao, metal.data(), metal.size(), &error ),
		    "metalness control uploads" );
		std::uint8_t metallicDark = 0;
		check( DrawWorld( context, &metallicDark, &error ), "metallic world pixel renders" );
		check( metallicDark + 15 < diffuse, "metalness removes baked diffuse response" );
		scene.lightRadiance[0] = scene.lightRadiance[1] = scene.lightRadiance[2] = 1.0f;
		context.SetDynamicPbrWorldScene( scene );
		std::uint8_t metallicLit = 0;
		check(
		    DrawWorld( context, &metallicLit, &error ), "metallic direct-specular pixel renders" );
		check( metallicLit > metallicDark + 15,
		    "scene light and camera produce a metal specular response" );
		check( context.UploadManagedTexture( mrao, roughMetal.data(), roughMetal.size(), &error ),
		    "roughness control uploads" );
		std::uint8_t roughLit = 0;
		check( DrawWorld( context, &roughLit, &error ), "rough metallic world pixel renders" );
		check( roughLit + 10 < metallicLit, "roughness broadens and dims the direct highlight" );
		check( context.UploadManagedTexture( mrao, metal.data(), metal.size(), &error ),
		    "metalness and roughness return to the reference state" );
		check( context.UploadManagedTexture( normal, normalTilt.data(), normalTilt.size(), &error ),
		    "tilted tangent-space normal uploads" );
		scene.material[1] = 1.0f;
		context.SetDynamicPbrWorldScene( scene );
		std::uint8_t tilted = 0;
		check( DrawWorld( context, &tilted, &error ), "normal-mapped world pixel renders" );
		check(
		    tilted + 15 < metallicLit, "packed WMSH tangent changes the direct specular response" );
		// Emission and $envmap. A smooth, black-diffuse metal with no light
		// isolates each term; the 1x1 lightmap carries no probe marker.
		check( context.UploadManagedTexture( normal, normalUp.data(), normalUp.size(), &error ),
		    "flat normal restored" );
		scene.material[1] = 0.0f;
		const int emission = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
		const int environment = context.CreateManagedTexture(
		    1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error, 0, 1, VK_FORMAT_UNDEFINED, true );
		check( emission >= 0 && environment >= 0, "emission and environment cube images" );
		const std::array<std::uint8_t, 4> emissive = { 100, 40, 10, 255 };
		const std::array<std::uint8_t, 4> black = { 0, 0, 0, 255 };
		const std::array<std::uint8_t, 4> smoothMetal = { 255, 10, 255, 255 };
		check( context.UploadManagedTexture( emission, emissive.data(), 4, &error ) &&
		           context.UploadManagedTexture( mrao, smoothMetal.data(), 4, &error ) &&
		           context.UploadManagedTexture( base, black.data(), 4, &error ),
		    "emission, smooth metal and black base upload" );
		for ( std::uint32_t face = 0; face < 6; ++face )
		{
			// Only +Z, which the flat surface reflects toward the eye, is bright.
			const std::array<std::uint8_t, 4> texel =
			    face == 4 ? std::array<std::uint8_t, 4>{ 60, 120, 240, 255 } : black;
			check( context.UploadManagedTexture( environment, texel.data(), 4, &error, 0, face ),
			    "environment cube face uploads" );
		}
		scene.lightRadiance[0] = scene.lightRadiance[1] = scene.lightRadiance[2] = 0.0f;
		render_vulkan::CVulkanContext::PbrWorldMaps maps;
		maps.emission = emission;
		maps.emissionScale = 1.5f;
		check( context.SelectPbrWorldMaterial( mrao, normal, eye, -1.0f, maps ),
		    "WMSH material selection binds an emission map" );
		std::uint8_t emitted = 0;
		check( DrawWorld( context, &emitted, &error ), "emissive world pixel renders" );
		// sRGB( decode( 100 / 255 ) * 1.5 ) = 121; the bytes as linear would be 149.
		check( emitted >= 119 && emitted <= 123,
		    "$emissiontexture is sRGB-decoded and scaled by $emissionscale" );
		render_vulkan::CVulkanContext::PbrWorldMaps cubeMaps;
		cubeMaps.environment = environment;
		check( !context.SelectPbrWorldMaterial( mrao, normal, eye, -1.0f,
		           render_vulkan::CVulkanContext::PbrWorldMaps{ -1, 0.0f, base } ),
		    "a 2D texture is rejected as the $envmap cube" );
		// A white metal (F0 = 1, directional albedo ~1) mirrors the linear
		// +Z texel: sRGB( 60 / 255 ) = 133. A sampled -Z face would be black.
		const std::array<std::uint8_t, 4> white = { 255, 255, 255, 255 };
		check( context.UploadManagedTexture( base, white.data(), 4, &error ),
		    "white metal base uploads" );
		check( context.SelectPbrWorldMaterial( mrao, normal, eye, -1.0f, cubeMaps ),
		    "WMSH material selection binds an $envmap cube" );
		std::uint8_t reflected = 0;
		check( DrawWorld( context, &reflected, &error ), "environment-mapped pixel renders" );
		std::fprintf( stderr, "envmap red %u (expected ~133)\n", reflected );
		check( reflected >= 128 && reflected <= 136,
		    "$envmap mirrors the reflected +Z face through the split-sum albedo" );
		// $clearcoat at a grazing view: eye far along +X, so N.V ~ 0.1 and the
		// coat's Fresnel ~ 0.61. Both layers reflect toward -X; a black, fully
		// rough dielectric base leaves env * ( albedo ( 1 - Fc ) + Fc ).
		for ( std::uint32_t face = 0; face < 6; ++face )
		{
			const std::array<std::uint8_t, 4> texel =
			    face == 1 ? std::array<std::uint8_t, 4>{ 200, 100, 50, 255 } : black;
			check( context.UploadManagedTexture( environment, texel.data(), 4, &error, 0, face ),
			    "environment cube face uploads" );
		}
		const std::array<std::uint8_t, 4> roughDielectric = { 0, 255, 255, 255 };
		check( context.UploadManagedTexture( base, black.data(), 4, &error ) &&
		           context.UploadManagedTexture( mrao, roughDielectric.data(), 4, &error ),
		    "black rough dielectric uploads" );
		const float grazingEye[3] = { 10.0f, 0.0f, 1.5f };
		std::uint8_t uncoated = 0;
		check( context.SelectPbrWorldMaterial( mrao, normal, grazingEye, -1.0f, cubeMaps ) &&
		           DrawWorld( context, &uncoated, &error ),
		    "uncoated grazing pixel renders" );
		render_vulkan::CVulkanContext::PbrWorldMaps coatMaps = cubeMaps;
		coatMaps.clearCoat = 1.0f;
		coatMaps.clearCoatRoughness = 0.05f;
		std::uint8_t coated = 0;
		check( context.SelectPbrWorldMaterial( mrao, normal, grazingEye, -1.0f, coatMaps ) &&
		           DrawWorld( context, &coated, &error ),
		    "clear-coated grazing pixel renders" );
		{
			const float normalDotView = 1.0f / std::sqrt( 101.0f );
			const float grazing = 1.0f - normalDotView;
			const float fresnel = 0.04f + 0.96f * std::pow( grazing, 5.0f );
			const render::pbr::SplitSumCoefficients split =
			    render::pbr::SampleSplitSum( normalDotView, 1.0f );
			const float albedo = std::min( 1.0f, 0.04f * split.a + split.b );
			const float linear = ( 200.0f / 255.0f ) * ( albedo * ( 1.0f - fresnel ) + fresnel );
			const float encoded =
			    linear <= 0.0031308f ? linear * 12.92f
			                         : 1.055f * std::pow( linear, 1.0f / 2.4f ) - 0.055f;
			std::fprintf( stderr, "clear coat red %u (expected %.1f), uncoated %u\n", coated,
			    encoded * 255.0f, uncoated );
			check( std::abs( coated - encoded * 255.0f ) <= 4.0f,
			    "$clearcoat adds its grazing Fresnel reflection over the attenuated base" );
			check( coated > uncoated + 40, "the coat is visible against the uncoated base" );
			check( !context.SelectPbrWorldMaterial( mrao, normal, grazingEye, -1.0f,
			           render_vulkan::CVulkanContext::PbrWorldMaps{ -1, 0.0f, -1, 1.5f, 0.1f } ),
			    "an out-of-range $clearcoat is rejected" );
		}
		check( context.UploadManagedTexture( base, baseBytes.data(), baseBytes.size(), &error ),
		    "base color restored" );
		context.DestroyManagedTexture( emission );
		context.DestroyManagedTexture( environment );
		context.ReleaseWorldMesh(); // also releases the map-scoped lightmap
		context.DestroyManagedTexture( base );
		context.DestroyManagedTexture( mrao );
		context.DestroyManagedTexture( normal );
	}
	context.Shutdown();
	SDL_DestroyWindow( window );
	SDL_Quit();
	return testing::ReportConformance( checks, failures );
}
