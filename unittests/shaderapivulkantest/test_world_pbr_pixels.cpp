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
