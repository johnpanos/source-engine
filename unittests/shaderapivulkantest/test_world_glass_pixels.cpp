//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0007 glass and readable scene depth on native Vulkan: WMSH
//          batches drawn with the transmissive PBRMetalRough pipeline over an
//          opaque WMSH scene, checked against the dielectric model's pixels
//          and the captured depth. Clip space is world space here (identity
//          transform, w = 1), so depth, refraction offsets and blur radii
//          follow from the vertex coordinates.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_surface_host.h"
#include "../../materialsystem/shaderapivulkan/vulkan_device.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace
{

using render_vulkan::CVulkanContext;

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

// Quads facing the eye (octahedral (1, 1) is -Z) with tangent +X.
void AddQuad( std::vector<WorldVertex> &vertices, std::vector<std::uint32_t> &indices, float x0,
    float x1, float z )
{
	const std::uint32_t base = static_cast<std::uint32_t>( vertices.size() );
	const float xs[4] = { x0, x1, x1, x0 };
	const float ys[4] = { -1.0f, -1.0f, 1.0f, 1.0f };
	for ( int i = 0; i < 4; ++i )
	{
		WorldVertex v = {};
		v.position[0] = xs[i];
		v.position[1] = ys[i];
		v.position[2] = z;
		v.normal[0] = v.normal[1] = 32767;
		v.tangent[0] = 32767;
		v.handedness = 1;
		// The background's base texture is red left of x = 0, blue right of it.
		v.uv[0] = 0.5f * ( xs[i] + 1.0f );
		v.uv[1] = 0.5f;
		v.lightmapUv[0] = v.lightmapUv[1] = 0.5f;
		vertices.push_back( v );
	}
	for ( std::uint32_t index : { 0u, 1u, 2u, 0u, 2u, 3u } )
		indices.push_back( base + index );
}

float SrgbToLinear( std::uint8_t value )
{
	const float c = value / 255.0f;
	return c <= 0.04045f ? c / 12.92f : std::pow( ( c + 0.055f ) / 1.055f, 2.4f );
}

struct Pixel
{
	float r, g, b; // linear
};

} // namespace

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SKIP: SDL video unavailable: %s\n", SDL_GetError() );
		return 77;
	}
	const int kSize = 128;
	SDL_Window *window = SDL_CreateWindow( "wmsh-glass-pixels", kSize, kSize, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "SKIP: Vulkan window unavailable: %s\n", SDL_GetError() );
		SDL_Quit();
		return 77;
	}
	std::string error;
	std::unique_ptr<render_vulkan::IVulkanSurfaceHost> host =
	    render_vulkan::MakeSdl3LegacySurfaceHost( window, &error );
	CVulkanContext context;
	render_vulkan::VulkanContextConfig config;
	config.appName = "wmsh-glass-pixels";
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
	check( context.InitDynamicMesh( &error ) && context.PbrGlassPipelineSupported(),
	    "WMSH glass pipeline initializes" );
	check( context.SceneDepthSupported(), "the depth format can be copied and sampled" );

	// Batches: background (z 0.8), glass (0.5), left-half foreground (0.3),
	// and a near background (0.55).
	std::vector<WorldVertex> vertices;
	std::vector<std::uint32_t> indices;
	AddQuad( vertices, indices, -1.0f, 1.0f, 0.8f );
	AddQuad( vertices, indices, -1.0f, 1.0f, 0.5f );
	AddQuad( vertices, indices, -1.0f, 0.0f, 0.3f );
	AddQuad( vertices, indices, -1.0f, 1.0f, 0.55f );
	const std::uint32_t kBackground = 0, kGlass = 6, kForeground = 12, kNearBackground = 18;
	check( context.UploadWorldMesh( vertices.data(), vertices.size() * sizeof( WorldVertex ),
	           indices.data(), indices.size() * sizeof( std::uint32_t ), &error ),
	    "glass fixture mesh uploads" );

	const int split = context.CreateManagedTexture( 2, 1, VK_FORMAT_R8G8B8A8_SRGB, &error );
	const int green = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_SRGB, &error );
	const int white = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_SRGB, &error );
	const int gray = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_SRGB, &error );
	const int matte = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	const int clear = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	const int frosted = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
	const int tilted = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8_UNORM, &error );
	const int lightmap =
	    context.CreateManagedTexture( 1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, &error );
	const bool imagesReady = split >= 0 && green >= 0 && white >= 0 && gray >= 0 && matte >= 0 &&
	                         clear >= 0 && frosted >= 0 && tilted >= 0 && lightmap >= 0;
	check( imagesReady, "glass fixture images are created" );
	if ( !imagesReady )
	{
		context.Shutdown();
		SDL_DestroyWindow( window );
		SDL_Quit();
		return testing::ReportConformance( checks, failures );
	}
	const std::array<std::uint8_t, 8> splitTexels = { 255, 0, 0, 255, 0, 0, 255, 255 };
	const std::array<std::uint8_t, 4> greenTexel = { 0, 255, 0, 255 };
	const std::array<std::uint8_t, 4> whiteTexel = { 255, 255, 255, 255 };
	const std::array<std::uint8_t, 4> grayTexel = { 128, 128, 128, 255 };
	const std::array<std::uint8_t, 4> matteTexel = { 0, 255, 255, 255 };   // rough dielectric
	const std::array<std::uint8_t, 4> clearTexel = { 0, 0, 255, 255 };     // smooth glass
	const std::array<std::uint8_t, 4> frostedTexel = { 0, 179, 255, 255 }; // roughness 0.7
	const std::array<std::uint8_t, 2> tiltedTexel = { 170, 128 };          // ~19.5 deg about Y
	const std::array<std::uint16_t, 4> one = { 0x3c00, 0x3c00, 0x3c00, 0x3c00 };
	check(
	    context.UploadManagedTexture( split, splitTexels.data(), splitTexels.size(), &error ) &&
	        context.UploadManagedTexture( green, greenTexel.data(), greenTexel.size(), &error ) &&
	        context.UploadManagedTexture( white, whiteTexel.data(), whiteTexel.size(), &error ) &&
	        context.UploadManagedTexture( gray, grayTexel.data(), grayTexel.size(), &error ) &&
	        context.UploadManagedTexture( matte, matteTexel.data(), matteTexel.size(), &error ) &&
	        context.UploadManagedTexture( clear, clearTexel.data(), clearTexel.size(), &error ) &&
	        context.UploadManagedTexture(
	            frosted, frostedTexel.data(), frostedTexel.size(), &error ) &&
	        context.UploadManagedTexture(
	            tilted, tiltedTexel.data(), tiltedTexel.size(), &error ) &&
	        context.UploadManagedTexture( lightmap,
	            reinterpret_cast<const std::uint8_t *>( one.data() ), sizeof( one ), &error ),
	    "glass fixture texels upload" );
	context.SetWorldLightmapHandle( lightmap );
	const float eye[3] = { 0.0f, 0.0f, -10.0f };

	CVulkanContext::DynRasterState opaque;
	opaque.cullMode = VK_CULL_MODE_NONE;
	CVulkanContext::DynRasterState glassRaster = opaque;
	glassRaster.depthWrite = false;
	const auto queueOpaque = [&]( int base, std::uint32_t firstIndex )
	{
		context.BindManagedTexture( base );
		context.SelectDynamicColorSpace(
		    CVulkanContext::kColorSrgbReadBase | CVulkanContext::kColorSrgbWrite );
		context.SelectDynamicRasterState( opaque );
		bool ok = context.SelectPbrWorldMaterial( matte, -1, eye, -1.0f );
		CVulkanContext::PbrWorldScene scene;
		scene.eye[0] = eye[0];
		scene.eye[1] = eye[1];
		scene.eye[2] = eye[2];
		scene.lightRadiance[0] = scene.lightRadiance[1] = scene.lightRadiance[2] = 0.0f;
		context.SetDynamicPbrWorldScene( scene );
		return ok && context.QueueWorldMeshBatch( firstIndex, 6 );
	};
	const auto queueGlass = [&](
	                            int tint, int mrao, int normal, float thickness, std::uint64_t key )
	{
		context.BindManagedTexture( tint );
		context.SelectDynamicColorSpace(
		    CVulkanContext::kColorSrgbReadBase | CVulkanContext::kColorSrgbWrite );
		context.SelectDynamicRasterState( glassRaster );
		CVulkanContext::PbrGlassParams params;
		params.thickness = thickness;
		params.materialKey = key;
		return context.SelectPbrGlassMaterial( mrao, normal, eye, -1.0f, params ) &&
		       context.QueueWorldMeshBatch( kGlass, 6 );
	};
	std::vector<std::uint8_t> frame;
	int frameW = 0, frameH = 0;
	const auto render = [&]( const std::function<bool()> &queue )
	{
		context.ClearDynamicQueue();
		context.SetClearColor( 0, 0, 0, 1 );
		if ( !queue() )
		{
			error = "a fixture draw was not accepted";
			return false;
		}
		context.RequestCapture();
		bool skip = false;
		if ( !context.BeginFrame( &skip, &error ) || skip || !context.EndFrame( &error ) )
			return false;
		frame = context.GetCapturedPixels( &frameW, &frameH );
		if ( frameW < kSize / 2 || frameH < kSize / 2 ||
		     frame.size() < size_t( frameW ) * size_t( frameH ) * 4 )
		{
			error = "incomplete capture";
			return false;
		}
		return true;
	};
	// A pixel of the middle row, `x` of 128 across (the frame may be scaled).
	const auto at = [&]( int x ) -> Pixel
	{
		const size_t column = size_t( x ) * size_t( frameW ) / kSize;
		const std::uint8_t *p = &frame[( size_t( frameH / 2 ) * frameW + column ) * 4];
		return { SrgbToLinear( p[0] ), SrgbToLinear( p[1] ), SrgbToLinear( p[2] ) };
	};
	const auto within = []( float value, float expected, float tolerance )
	{
		return std::fabs( value - expected ) <= tolerance;
	};

	// Reference: the background alone.
	std::vector<float> depth;
	std::uint32_t depthW = 0, depthH = 0;
	check( !context.ReadSceneDepth( &depth, &depthW, &depthH, &error ),
	    "scene depth is unreadable before any capture" );
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground );
	           } ),
	    "background renders without glass" );
	const Pixel redBackground = at( 16 );
	const Pixel blueBackground = at( kSize - 16 );
	check( redBackground.r > 0.5f && redBackground.b < 0.02f && blueBackground.b > 0.5f &&
	           blueBackground.r < 0.02f,
	    "background is red on the left and blue on the right" );
	check( context.LastFrameSceneCaptures() == 0, "a frame without glass captures nothing" );

	// Clear thin glass: a sheet reflects 2E / (1 + E), E the interface's
	// split-sum reflectance at normal incidence (about 0.04 at IOR 1.5), and
	// passes the rest. With no probe nothing is reflected back.
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground ) &&
		                  queueGlass( white, clear, -1, 0.0f, 1 );
	           } ),
	    "clear glass renders over the background" );
	const float sheet = 1.0f - 2.0f * 0.04f / 1.04f;
	check( within( at( 16 ).r, redBackground.r * sheet, 0.02f ) &&
	           within( at( kSize - 16 ).b, blueBackground.b * sheet, 0.02f ),
	    "clear glass transmits the scene behind it less the sheet's reflectance" );
	check( at( 16 ).r < redBackground.r - 0.02f, "glass is not an invisible pass-through" );
	check( context.LastFrameSceneCaptures() == 1 && context.LastFrameSceneDepthCaptures() == 1,
	    "one glass material captures color and depth once" );

	// Readable scene depth: the capture before the glass holds the background.
	check( context.ReadSceneDepth( &depth, &depthW, &depthH, &error ) &&
	           depthW >= std::uint32_t( frameW ) && depthH >= std::uint32_t( frameH ) &&
	           within( depth[size_t( frameH / 2 ) * depthW + frameW / 2], 0.8f, 1e-4f ),
	    "scene depth reads back the background's depth, not the glass's" );

	// Tint: the base color multiplies transmitted light.
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground ) &&
		                  queueGlass( gray, clear, -1, 0.0f, 1 );
	           } ),
	    "tinted glass renders" );
	check( within( at( 16 ).r, redBackground.r * sheet * SrgbToLinear( 128 ), 0.02f ),
	    "base color tints transmitted light" );

	// Refraction: 2 units of solid glass tilted about Y bend the ray -X by
	// about 15 pixels, so blue pixels just right of the seam show red.
	const int seamRight = kSize / 2 + 6;
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground ) &&
		                  queueGlass( white, clear, tilted, 0.0f, 1 );
	           } ),
	    "thin tilted glass renders" );
	check( at( seamRight ).b > 0.5f && at( seamRight ).r < 0.02f,
	    "a thin sheet passes light straight through" );
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground ) &&
		                  queueGlass( white, clear, tilted, 2.0f, 1 );
	           } ),
	    "thick tilted glass renders" );
	check( at( seamRight ).r > 0.5f && at( seamRight ).b < 0.02f,
	    "thick glass refracts the scene behind it by Snell's law" );
	check( at( kSize - 16 ).b > 0.5f, "refraction offsets are local, not a global shift" );

	// Foreground rejection: an opaque object in front of the glass must not
	// appear through it. Negative control: without depth it does.
	const auto foregroundScene = [&]
	{
		return queueOpaque( split, kBackground ) && queueOpaque( green, kForeground ) &&
		       queueGlass( white, clear, tilted, 2.0f, 1 );
	};
	check( render( foregroundScene ), "glass behind a foreground object renders" );
	check( at( seamRight ).g < 0.05f && at( seamRight ).b > 0.5f,
	    "scene depth rejects refracted samples of nearer geometry" );
	check( at( 16 ).g > 0.5f, "the foreground object itself is drawn over the glass" );
	context.SetSceneDepthEnabled( false );
	check( render( foregroundScene ), "glass without scene depth renders" );
	check( at( seamRight ).g > 0.5f && context.LastFrameSceneDepthCaptures() == 0,
	    "negative control: without depth, the foreground leaks through the glass" );
	context.SetSceneDepthEnabled( true );

	// Rough glass blurs by its GGX spread over the distance to what is behind
	// it: the far background blurs the red/blue seam, a near one much less.
	const int seamBlue = kSize / 2 + 2;
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground ) &&
		                  queueGlass( white, frosted, -1, 0.0f, 1 );
	           } ),
	    "frosted glass over a far background renders" );
	const Pixel farBlur = at( seamBlue );
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kNearBackground ) &&
		                  queueGlass( white, frosted, -1, 0.0f, 1 );
	           } ),
	    "frosted glass over a near background renders" );
	const Pixel nearBlur = at( seamBlue );
	check( farBlur.r > 0.05f && farBlur.r > nearBlur.r + 0.03f,
	    "rough glass blurs more the farther the scene is behind it" );
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground ) &&
		                  queueGlass( white, clear, -1, 0.0f, 1 );
	           } ),
	    "clear glass at the seam renders" );
	check( at( seamBlue ).r < 0.02f, "smooth glass does not blur" );

	// Glass behind glass: a second material recaptures, so the nearer sheet
	// sees the farther one; the same material again does not.
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground ) &&
		                  queueGlass( white, clear, -1, 0.0f, 1 ) &&
		                  queueGlass( white, clear, -1, 0.0f, 2 );
	           } ),
	    "two glass materials render" );
	check( context.LastFrameSceneCaptures() == 2 &&
	           within( at( 16 ).r, redBackground.r * sheet * sheet, 0.02f ),
	    "a second glass material refracts the first" );
	check( render(
	           [&]
	           {
		           return queueOpaque( split, kBackground ) &&
		                  queueGlass( white, clear, -1, 0.0f, 1 ) &&
		                  queueGlass( white, clear, -1, 0.0f, 1 );
	           } ),
	    "one glass material in two draws renders" );
	check( context.LastFrameSceneCaptures() == 1, "draws of one glass material share a capture" );

	// Invalid glass values are refused before anything is queued.
	CVulkanContext::PbrGlassParams invalid;
	invalid.ior = 0.5f;
	context.BindManagedTexture( white );
	check( !context.SelectPbrGlassMaterial( clear, -1, eye, -1.0f, invalid ),
	    "an index of refraction below 1 is rejected" );

	context.ReleaseWorldMesh();
	for ( int handle : { split, green, white, gray, matte, clear, frosted, tilted } )
		context.DestroyManagedTexture( handle );
	context.Shutdown();
	SDL_DestroyWindow( window );
	SDL_Quit();
	return testing::ReportConformance( checks, failures );
}
