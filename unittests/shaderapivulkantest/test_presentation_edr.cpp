//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Device oracle for render.presentation.v1 "Dynamic range" on the
//          SDL3-Vulkan pair (render.presentation.edr). On an iPhone or Apple TV
//          through MoltenVK, an extended-linear presentation must:
//          - create a half-float swapchain in the extended linear sRGB color
//            space;
//          - keep values above SDR white in the presented image;
//          - put the Metal layer in high dynamic range;
//          - report the screen's headroom, which rises above 1 once EDR is
//            engaged.
//          A standard presentation on the same window is the control. It
//          clips at white and leaves the layer standard.
//          Runs through tools/quality/ios_conformance.py; the frames it
//          presents alternate SDR white and brighter-than-white, so a person
//          watching the screen sees the difference.
//
//===========================================================================//

#include "../../render/bridge/sdl3-vulkan/sdl3_vulkan_presentation.h"
#include "../../render/device/vulkan/backend_v1/render_backend_v1.h"
#include "../../platform/sdl3/render_surface/sdl3_render_surfaces.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

using namespace render;

namespace
{

unsigned long g_Checks = 0;
unsigned long g_Failures = 0;

void Check( bool ok, const char *id, const std::string &detail )
{
	++g_Checks;
	if ( !ok )
		++g_Failures;
	std::printf( "%s %s: %s\n", ok ? "ok  " : "FAIL", id, detail.c_str() );
}

struct Rig
{
	platform_sdl3::Sdl3RenderSurfaces &surfaces;
	render_vulkan::Sdl3VulkanPresentationBridge &bridge;
	render_vulkan::VulkanRenderBackend &provider;
	IRenderDevice &device;

	void Pump()
	{
		SDL_PumpEvents();
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
			surfaces.HandleEvent( event );
	}

	// One frame whose back buffer is cleared to 'value' (all three channels).
	// With 'capture', the presented swapchain image is kept for ReadCapture*.
	bool PresentValue( IRenderPresentation &p, float value, bool capture )
	{
		for ( int attempt = 0; attempt < 6; ++attempt )
		{
			Pump();
			RenderResourceHandle back = kInvalidResource;
			const RenderPresentStatus begin = p.BeginFrame( &back );
			if ( begin == RenderPresentStatus::kRecoverable ||
			     begin == RenderPresentStatus::kSuspended )
				continue;
			if ( begin != RenderPresentStatus::kOk )
				return false;
			IRenderCommandContext *context = device.CreateCommandContext();
			context->RecordUse( back );
			render_vulkan::VulkanDeviceEndpoint *ep = provider.FindDevice( device );
			VkClearColorValue color = {};
			color.float32[0] = color.float32[1] = color.float32[2] = value;
			color.float32[3] = 1.0f;
			VkImageSubresourceRange range = {};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.levelCount = 1;
			range.layerCount = 1;
			vkCmdClearColorImage( ep->CommandBuffer( *context ), ep->Image( back ),
			    VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range );
			device.Submit( *context );
			if ( capture )
				bridge.RequestCapture( p );
			const RenderPresentStatus status = p.Present();
			if ( status == RenderPresentStatus::kOk )
				return true;
			if ( status != RenderPresentStatus::kRecoverable )
				return false;
		}
		return false;
	}
};

// Every sampled pixel of a linear capture has RGB within 'tolerance' of
// 'value'.
bool LinearCaptureIs( const std::vector<float> &rgba, uint32_t w, uint32_t h, float value,
    float tolerance, float *outSample )
{
	if ( w == 0 || h == 0 || rgba.size() < size_t( w ) * h * 4 )
		return false;
	const uint32_t xs[] = { 0, w / 2, w - 1 };
	const uint32_t ys[] = { 0, h / 2, h - 1 };
	*outSample = rgba[( size_t( h / 2 ) * w + w / 2 ) * 4];
	for ( uint32_t y : ys )
		for ( uint32_t x : xs )
			for ( int c = 0; c < 3; ++c )
				if ( std::fabs( rgba[( size_t( y ) * w + x ) * 4 + c] - value ) > tolerance )
					return false;
	return true;
}

std::string Describe( const RenderDynamicRangeState &state )
{
	char text[96];
	std::snprintf( text, sizeof( text ), "%s, headroom %.2f of %.2f",
	    state.range == RenderDynamicRange::kExtendedLinear ? "extended-linear" : "standard",
	    state.currentHeadroom, state.potentialHeadroom );
	return text;
}

void RunChecks( Rig &rig, IRenderSurface &surface )
{
	RenderPresentationConfig config;
	config.format = RenderColorFormat::kRGBA16Float;
	config.dynamicRange = RenderDynamicRange::kExtendedLinear;
	RenderCreateError error;
	IRenderPresentation *p = rig.bridge.CreatePresentation( rig.device, surface, config, &error );
	Check( p != nullptr, "edr.create",
	    p ? "an extended-linear kRGBA16Float presentation is created"
	      : std::string( "creation failed: " ) + error.message );
	if ( !p )
		return;

	RenderDynamicRangeState state = p->GetDynamicRange();
	Check( state.range == RenderDynamicRange::kExtendedLinear && state.potentialHeadroom > 1.0f,
	    "edr.display_headroom",
	    "the presentation is extended-linear on a display with headroom (" + Describe( state ) +
	        ")" );

	// Four times SDR white survives to the presented image.
	std::vector<float> linear;
	uint32_t w = 0, h = 0;
	float sample = 0.0f;
	const bool presented = rig.PresentValue( *p, 4.0f, true );
	const bool read = presented && rig.bridge.ReadCaptureLinear( *p, &linear, &w, &h );
	const bool above = read && LinearCaptureIs( linear, w, h, 4.0f, 0.01f, &sample );
	Check( above, "edr.values_above_white",
	    read ? "the presented half-float image holds 4.0 (sampled " + std::to_string( sample ) +
	               ", " + std::to_string( w ) + "x" + std::to_string( h ) + ")"
	         : std::string( "no linear capture of the presented frame" ) );
	const bool readLow = rig.PresentValue( *p, 0.5f, true ) &&
	                     rig.bridge.ReadCaptureLinear( *p, &linear, &w, &h );
	Check( readLow && LinearCaptureIs( linear, w, h, 0.5f, 0.01f, &sample ) &&
	           !LinearCaptureIs( linear, w, h, 4.0f, 0.01f, &sample ),
	    "edr.capture_control", "a 0.5 frame reads 0.5 and not 4.0: the capture tells values apart" );

	bool layerExtended = false, extendedLinearSpace = false;
	const bool known = rig.bridge.ReadNativeDynamicRange( *p, &layerExtended, &extendedLinearSpace );
	Check( known && layerExtended && extendedLinearSpace, "edr.layer",
	    std::string( "the Metal layer shows high dynamic range in extended linear sRGB (known " ) +
	        ( known ? "yes" : "no" ) + ", high " + ( layerExtended ? "yes" : "no" ) +
	        ", extended linear " + ( extendedLinearSpace ? "yes" : "no" ) + ")" );

	// EDR ramps up over a few seconds after the layer asks for it. Alternate SDR
	// white and the brightest value the display offers, a second each.
	float bestHeadroom = 1.0f;
	const uint64_t start = SDL_GetTicks();
	while ( SDL_GetTicks() - start < 6000 )
	{
		state = p->GetDynamicRange();
		bestHeadroom = std::max( bestHeadroom, state.currentHeadroom );
		const bool bright = ( ( SDL_GetTicks() - start ) / 1000 ) % 2 == 1;
		rig.PresentValue( *p, bright ? std::max( 1.0f, state.potentialHeadroom ) : 1.0f, false );
	}
	state = p->GetDynamicRange();
	std::printf( "render.presentation.edr: after the ramp %s, highest current headroom %.2f\n",
	    Describe( state ).c_str(), bestHeadroom );
	Check( bestHeadroom > 1.0f, "edr.headroom_engaged",
	    "the screen's current headroom rises above 1 while extended frames present (highest " +
	        std::to_string( bestHeadroom ) + ")" );
	rig.bridge.DestroyPresentation( p );
	while ( rig.device.PollCompletion() > 0 )
	{
	}

	// Control: a standard presentation on the same window clips at white and
	// returns the layer to standard range.
	IRenderPresentation *standard =
	    rig.bridge.CreatePresentation( rig.device, surface, RenderPresentationConfig(), nullptr );
	std::vector<uint8_t> rgba8;
	const bool standardRead = standard && rig.PresentValue( *standard, 4.0f, true ) &&
	                          rig.bridge.ReadCapture( *standard, &rgba8, &w, &h );
	const size_t centre = ( size_t( h / 2 ) * w + w / 2 ) * 4;
	Check( standardRead && rgba8.size() > centre && rgba8[centre] == 255 &&
	           standard->GetDynamicRange().currentHeadroom == 1.0f,
	    "edr.standard_control",
	    "a standard presentation clips 4.0 to white and reports headroom 1" );
	layerExtended = true;
	const bool standardKnown =
	    standard && rig.bridge.ReadNativeDynamicRange( *standard, &layerExtended, &extendedLinearSpace );
	Check( standardKnown && !layerExtended, "edr.standard_layer",
	    "the layer returns to standard range for a standard presentation" );
	if ( standard )
		rig.bridge.DestroyPresentation( standard );
}

} // namespace

int main()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::printf( "render.presentation.edr: SDL_Init failed: %s\n", SDL_GetError() );
		return testing::ReportConformance( 1, 1 );
	}
	render_vulkan::VulkanProviderOptions options;
	std::string error;
	const bool haveExtensions =
	    render_vulkan::Sdl3VulkanInstanceExtensions( &options.instanceExtensions, &error );
	const bool colorspace =
	    std::find( options.instanceExtensions.begin(), options.instanceExtensions.end(),
	        std::string( VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME ) ) !=
	    options.instanceExtensions.end();
	Check( haveExtensions && colorspace, "edr.colorspace_extension",
	    "the instance enables VK_EXT_swapchain_colorspace" );
	options.enableSwapchain = true;
	std::unique_ptr<render_vulkan::VulkanRenderBackend> provider =
	    haveExtensions ? render_vulkan::MakeVulkanRenderBackend( options, &error ) : nullptr;
	Check( provider != nullptr, "edr.provider", provider ? "Vulkan provider up" : error );
	if ( provider )
	{
		platform_sdl3::Sdl3RenderSurfaces surfaces;
		render_vulkan::Sdl3VulkanPresentationBridge bridge( *provider, surfaces, 2 );
		RenderDeviceRequest request;
		IRenderDevice *device = provider->CreateDevice( request, nullptr );
		SDL_Window *window = SDL_CreateWindow( "render.presentation.edr", 640, 480,
		    SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY );
		Check( device && window, "edr.setup", "a device and a window" );
		IRenderSurface *surface = nullptr;
		if ( device && window )
		{
			SDL_SyncWindow( window );
			surface = surfaces.Adopt( window );
			Rig rig{ surfaces, bridge, *provider, *device };
			rig.Pump();
			RunChecks( rig, *surface );
			while ( device->PollCompletion() > 0 )
			{
			}
		}
		// The window goes after its surface releases every native object on it.
		if ( surface )
			surfaces.InvalidateWindow( *surface );
		if ( window )
			SDL_DestroyWindow( window );
		if ( surface )
			surfaces.Destroy( surface );
		if ( device )
		{
			bridge.ReleaseDevice( *device );
			provider->DestroyDevice( device );
		}
	}
	provider.reset();
	SDL_Quit();
	std::printf( "render.presentation.edr: %lu check(s), %lu failure(s)\n", g_Checks, g_Failures );
	return testing::ReportConformance( g_Checks, g_Failures );
}
