//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Real WMSH vertex, material and baked-light inputs through the
//          scene-derived native Vulkan PBR world shader.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_surface_host.h"
#include "../../materialsystem/shaderapivulkan/vulkan_device.h"
#include "../../materialsystem/shaderapivulkan/vulkan_world_lightmap.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <cstdio>
#ifdef RFC0008_KTX_READER
#include <fstream>
#include <iterator>
#endif
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
#ifndef RFC0008_KTX_READER
	const int lightmap =
	    context.CreateManagedTexture( 1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, &error );
	imagesReady = imagesReady && lightmap >= 0;
#endif
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
#ifdef RFC0008_KTX_READER
		std::ifstream package( "quality/fixtures/ktx2/white-1x1-rgba16f.ktx2", std::ios::binary );
		std::vector<char> packageBytes(
		    std::istreambuf_iterator<char>{ package }, std::istreambuf_iterator<char>{} );
		check( !packageBytes.empty(), "linear HDR lightmap KTX2 fixture exists" );
		if ( !packageBytes.empty() )
		{
			auto damaged = packageBytes;
			damaged[0] = 0;
			check( !render_vulkan::UploadWorldLightmapKtx2(
			           context, damaged.data(), damaged.size(), nullptr, nullptr, &error ),
			    "corrupt BSP2 lightmap package is rejected" );
			std::uint32_t width = 0;
			std::uint32_t height = 0;
			error.clear();
			check( render_vulkan::UploadWorldLightmapKtx2( context, packageBytes.data(),
			           packageBytes.size(), &width, &height, &error ) &&
			           width == 1 && height == 1,
			    "BSP2 lightmap KTX2 uploads as the map-scoped HDR image" );
		}
#else
		const std::array<std::uint16_t, 4> irradiance = { 0x3c00, 0x3c00, 0x3c00, 0x3c00 };
		check( context.UploadManagedTexture( lightmap,
		           reinterpret_cast<const std::uint8_t *>( irradiance.data() ),
		           sizeof( irradiance ), &error ),
		    "linear irradiance texels upload" );
		context.SetWorldLightmapHandle( lightmap );
#endif
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
