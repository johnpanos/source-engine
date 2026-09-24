//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GPU-runner sensitivity (RFC 0011 G0). Brings up the native Vulkan
//          device exactly as the pixel suites do, renders and reads back one
//          frame, then writes a zero-check result record and exits 0. The
//          conformance runner must classify this run as a failure ("zero
//          checks executed"): a GPU suite that ran no checks certifies
//          nothing, even with exit status 0.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_surface_host.h"
#include "../../materialsystem/shaderapivulkan/vulkan_device.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <memory>
#include <string>

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SDL video unavailable: %s\n", SDL_GetError() );
		return 2;
	}
	SDL_Window *window = SDL_CreateWindow( "gpu-zero-checks", 64, 64, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "Vulkan window unavailable: %s\n", SDL_GetError() );
		SDL_Quit();
		return 2;
	}
	std::string error;
	std::unique_ptr<render_vulkan::IVulkanSurfaceHost> host =
	    render_vulkan::MakeSdl3LegacySurfaceHost( window, &error );
	render_vulkan::CVulkanContext context;
	render_vulkan::VulkanContextConfig config;
	config.appName = "gpu-zero-checks";
	if ( !host || !context.Init( *host, config, &error ) )
	{
		std::fprintf( stderr, "native Vulkan context unavailable: %s\n", error.c_str() );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 2;
	}
	// One real frame, so the zero-check record follows genuine GPU work.
	bool skip = false;
	context.SetClearColor( 0, 0, 0, 1 );
	context.RequestCapture();
	const bool rendered = context.BeginFrame( &skip, &error ) && ( skip || context.EndFrame( &error ) );
	std::printf( "rendered one frame: %s\n", rendered ? "yes" : error.c_str() );
	context.Shutdown();
	host.reset();
	SDL_DestroyWindow( window );
	SDL_Quit();
	// Deliberately no checks, and a success exit status: only the result
	// protocol can reject this run.
	testing::ReportConformance( 0, 0 );
	return 0;
}
