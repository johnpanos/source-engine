//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0007 synthetic direct-light pixel oracle. Draws the native
//          layered PBR pipeline, captures a real Vulkan frame and compares
//          its linear output with the headless BRDF model. Base color is sampled
//          through an sRGB view while the packed MRAO texture stays linear.
//          Requires a native Vulkan device and a display; exit 77 means the
//          gate is unverified.
//
//===========================================================================//

#include "render/pbr_brdf.h"
#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_surface_host.h"
#include "../../materialsystem/shaderapivulkan/vulkan_device.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace
{
int g_checks = 0;
int g_failures = 0;

void Check( bool condition, const char *message )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", message );
	}
}

// The Source dynamic vertex stream is position, vertex color, base UV.
constexpr float kQuad[6][8] = {
    { -0.5f, -0.5f, 0.5f, 1, 1, 1, 0, 0 },
    { 0.5f, 0.5f, 0.5f, 1, 1, 1, 1, 1 },
    { 0.5f, -0.5f, 0.5f, 1, 1, 1, 1, 0 },
    { -0.5f, -0.5f, 0.5f, 1, 1, 1, 0, 0 },
    { -0.5f, 0.5f, 0.5f, 1, 1, 1, 0, 1 },
    { 0.5f, 0.5f, 0.5f, 1, 1, 1, 1, 1 },
};

struct PixelCase
{
	const char *name;
	uint8_t base[4];
	uint8_t mrao[4];
	float angles[4]; // N.V, N.L, N.H, V.H
	float radiance[4];
	bool roughnessControl;
	bool fresnelControl;
	bool baseColorControl;
	bool mraoLinearControl;
	bool diffuseControl;
};

float DecodeSrgb( uint8_t encoded )
{
	const float value = encoded / 255.0f;
	return value <= 0.04045f ? value / 12.92f : std::pow( ( value + 0.055f ) / 1.055f, 2.4f );
}

bool RunCase( render_vulkan::CVulkanContext &context, int baseHandle, int mraoHandle,
    const PixelCase &testCase, std::string &error )
{
	context.ClearDynamicQueue();
	context.SetClearColor( 0, 0, 0, 1 );
	if ( !context.UploadManagedTexture(
	         baseHandle, testCase.base, sizeof( testCase.base ), &error ) ||
	     !context.UploadManagedTexture(
	         mraoHandle, testCase.mrao, sizeof( testCase.mrao ), &error ) )
		return false;
	context.SelectDynamicShader( render_vulkan::CVulkanContext::kDynShaderPbrDirect );
	context.BindManagedTexture( baseHandle );
	context.BindManagedSampler( 1, mraoHandle );
	context.SelectDynamicColorSpace( render_vulkan::CVulkanContext::kColorSrgbReadBase );
	context.SetDynamicModulation( testCase.radiance );
	context.SetDynamicPbrAngles( testCase.angles );
	context.QueueDynamicTriangles( &kQuad[0][0], 6 );
	context.RequestCapture();
	bool skip = false;
	if ( !context.BeginFrame( &skip, &error ) || skip || !context.EndFrame( &error ) )
	{
		if ( skip )
			error = "surface unexpectedly skipped a PBR frame";
		return false;
	}
	int width = 0;
	int height = 0;
	const std::vector<uint8_t> &pixels = context.GetCapturedPixels( &width, &height );
	if ( width < 4 || height < 4 || pixels.size() < static_cast<size_t>( width * height * 4 ) )
	{
		error = "PBR frame capture was empty or incomplete";
		return false;
	}
	const uint8_t *center = &pixels[( static_cast<size_t>( height / 2 ) * width + width / 2 ) * 4];
	const uint8_t *corner = &pixels[0];
	Check( corner[0] <= 2 && corner[1] <= 2 && corner[2] <= 2,
	    "frame corner retains the black clear" );

	const render::pbr::Color base = { DecodeSrgb( testCase.base[0] ),
	    DecodeSrgb( testCase.base[1] ), DecodeSrgb( testCase.base[2] ) };
	const float metalness = testCase.mrao[0] / 255.0f;
	const float roughness = std::max( 0.02f, testCase.mrao[1] / 255.0f );
	const render::pbr::Color f0 = { 0.04f * ( 1.0f - metalness ) + base.red * metalness,
	    0.04f * ( 1.0f - metalness ) + base.green * metalness,
	    0.04f * ( 1.0f - metalness ) + base.blue * metalness };
	const render::pbr::Color brdf = render::pbr::EvaluateLayeredDirect( base, metalness,
	    testCase.angles[0], testCase.angles[1], testCase.angles[2], testCase.angles[3], roughness );
	const float expected[3] = { testCase.radiance[0] * brdf.red * testCase.angles[1],
	    testCase.radiance[1] * brdf.green * testCase.angles[1],
	    testCase.radiance[2] * brdf.blue * testCase.angles[1] };
	std::fprintf( stderr, "%s: GPU=(%u,%u,%u) CPU=(%.1f,%.1f,%.1f)\n", testCase.name, center[0],
	    center[1], center[2], expected[0] * 255.0f, expected[1] * 255.0f, expected[2] * 255.0f );
	for ( int channel = 0; channel < 3; ++channel )
	{
		Check(
		    std::abs( static_cast<float>( center[channel] ) - expected[channel] * 255.0f ) <= 4.0f,
		    "native Vulkan PBR pixel matches the headless BRDF" );
	}
	if ( testCase.roughnessControl )
	{
		// A seeded variant that uses alpha = roughness instead of roughness^2
		// predicts a much dimmer normal-incidence highlight for this case.
		const float wrongAlphaSquared = roughness * roughness;
		const float wrongDenominator =
		    testCase.angles[2] * testCase.angles[2] * ( wrongAlphaSquared - 1.0f ) + 1.0f;
		const float wrongDistribution =
		    wrongAlphaSquared / ( render::pbr::kPi * wrongDenominator * wrongDenominator );
		const float distribution = render::pbr::GgxDistribution( testCase.angles[2], roughness );
		const float wrongPixel = expected[0] * ( wrongDistribution / distribution ) * 255.0f;
		Check( std::abs( static_cast<float>( center[0] ) - wrongPixel ) > 20.0f,
		    "pixel oracle rejects the wrong roughness mapping" );
	}
	if ( testCase.fresnelControl )
	{
		// A constant F0 omits grazing Fresnel; the captured highlight must
		// be separated from that seeded output by more than quantization noise.
		const render::pbr::Color specular = render::pbr::EvaluateSpecular( f0, testCase.angles[0],
		    testCase.angles[1], testCase.angles[2], testCase.angles[3], roughness );
		const float wrongPixel =
		    testCase.radiance[0] *
		    ( brdf.red - specular.red +
		        f0.red * render::pbr::GgxDistribution( testCase.angles[2], roughness ) *
		            render::pbr::SmithVisibility(
		                testCase.angles[0], testCase.angles[1], roughness ) ) *
		    testCase.angles[1] * 255.0f;
		Check( std::abs( static_cast<float>( center[0] ) - wrongPixel ) > 20.0f,
		    "pixel oracle rejects missing grazing Fresnel" );
	}
	if ( testCase.baseColorControl )
	{
		// Sampling the sRGB base through the linear view would use its stored
		// byte value as F0. The colored metal separates the two paths visibly.
		const render::pbr::Color wrongBase = { testCase.base[0] / 255.0f, base.green, base.blue };
		const float wrongPixel =
		    testCase.radiance[0] *
		    render::pbr::EvaluateLayeredDirect( wrongBase, metalness, testCase.angles[0],
		        testCase.angles[1], testCase.angles[2], testCase.angles[3], roughness )
		        .red *
		    testCase.angles[1] * 255.0f;
		Check( std::abs( static_cast<float>( center[0] ) - wrongPixel ) > 20.0f,
		    "pixel oracle rejects linear sampling of the sRGB base" );
	}
	if ( testCase.mraoLinearControl )
	{
		// Metalness is data, not display color. Decoding the stored byte as
		// sRGB changes F0 and must be visible in the captured red channel.
		const float wrongMetalness = DecodeSrgb( testCase.mrao[0] );
		const float wrongPixel =
		    testCase.radiance[0] *
		    render::pbr::EvaluateLayeredDirect( base, wrongMetalness, testCase.angles[0],
		        testCase.angles[1], testCase.angles[2], testCase.angles[3], roughness )
		        .red *
		    testCase.angles[1] * 255.0f;
		Check( std::abs( static_cast<float>( center[0] ) - wrongPixel ) > 20.0f,
		    "pixel oracle rejects sRGB decoding of linear MRAO" );
	}
	if ( testCase.diffuseControl )
	{
		const float specularOnly =
		    testCase.radiance[0] *
		    render::pbr::EvaluateSpecular( f0, testCase.angles[0], testCase.angles[1],
		        testCase.angles[2], testCase.angles[3], roughness )
		        .red *
		    testCase.angles[1] * 255.0f;
		Check( std::abs( static_cast<float>( center[0] ) - specularOnly ) > 20.0f,
		    "pixel oracle rejects omitted layered diffuse" );
	}
	Check( center[3] >= 252, "PBR pixel is opaque" );
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
	SDL_Window *window = SDL_CreateWindow( "pbr-native-pixels", 128, 128, SDL_WINDOW_VULKAN );
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
	config.appName = "pbr-native-pixels";
	config.enableValidation = true;
	if ( !host || !context.Init( *host, config, &error ) )
	{
		std::fprintf( stderr, "SKIP: native Vulkan context unavailable: %s\n", error.c_str() );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 77;
	}
	if ( !context.InitDynamicMesh( &error ) || !context.PbrDirectPipelineSupported() )
	{
		std::fprintf( stderr, "FAIL: PBR pipeline unavailable: %s\n", error.c_str() );
		++g_failures;
	}
	else
	{
		const int base = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
		const int mrao = context.CreateManagedTexture( 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &error );
		Check( base >= 0 && mrao >= 0, "base and MRAO textures were created" );
		if ( base >= 0 && mrao >= 0 )
		{
			const PixelCase cases[] = {
			    { "dielectric-half-rough", { 255, 255, 255, 255 }, { 0, 128, 255, 255 },
			        { 1, 1, 1, 1 }, { 0.8f, 0.8f, 0.8f, 1 }, false, false, false, false, true },
			    { "metal-half-rough", { 255, 255, 255, 255 }, { 255, 128, 255, 255 },
			        { 1, 1, 1, 1 }, { 0.5f, 0.5f, 0.5f, 1 }, true, false, false, false, false },
			    { "colored-metal", { 204, 51, 26, 255 }, { 255, 255, 255, 255 }, { 1, 1, 1, 1 },
			        { 10, 10, 10, 1 }, false, false, true, false, false },
			    { "half-metal", { 255, 0, 0, 255 }, { 128, 255, 128, 255 }, { 1, 1, 1, 1 },
			        { 2, 2, 2, 1 }, false, false, false, true, false },
			    { "grazing-dielectric", { 255, 255, 255, 255 }, { 0, 255, 0, 255 },
			        { 0.2f, 0.2f, 1, 0.2f }, { 4, 4, 4, 1 }, false, true, false, false, false },
			};
			for ( const PixelCase &testCase : cases )
			{
				if ( !RunCase( context, base, mrao, testCase, error ) )
				{
					std::fprintf( stderr, "FAIL: %s: %s\n", testCase.name, error.c_str() );
					++g_failures;
				}
			}
		}
	}
	if ( context.ValidationEnabled() )
		Check( context.ValidationErrorCount() == 0, "no Vulkan validation errors" );
	else
		std::fprintf( stderr, "Vulkan validation layer unavailable; validation gate unverified\n" );
	context.Shutdown();
	host.reset();
	SDL_DestroyWindow( window );
	SDL_Quit();
	std::printf( "PBR native pixels: %d checks, %d failures\n", g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
