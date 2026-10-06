//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dynamic range for the SDL3-Vulkan bridge on platforms other than
//          iOS and tvOS (see sdl3_dynamic_range.h). The compositor takes the
//          swapchain's color space as is; SDL reports the window's headroom.
//
//===========================================================================//

#if defined( __APPLE__ )
#include <TargetConditionals.h>
#endif

// UIKit platforms (iOS, tvOS) have sdl3_dynamic_range_apple.mm.
#if !defined( __APPLE__ ) || !TARGET_OS_IPHONE

#include "sdl3_dynamic_range.h"

#include <SDL3/SDL.h>

namespace render_vulkan
{

bool Sdl3CanShowExtendedRange( SDL_Window *window )
{
	return window != nullptr;
}

bool Sdl3SetExtendedRange( SDL_Window *window, bool extended )
{
	(void)extended;
	return window != nullptr;
}

Sdl3DisplayHeadroom Sdl3ReadHeadroom( SDL_Window *window )
{
	Sdl3DisplayHeadroom headroom;
	if ( !window )
		return headroom;
	const float reported = SDL_GetFloatProperty(
	    SDL_GetWindowProperties( window ), SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT, 1.0f );
	headroom.current = reported > 1.0f ? reported : 1.0f;
	headroom.potential = headroom.current;
	// SDL gives SDR white as a multiple of 80 cd/m^2 (scRGB's unit), 1 where
	// the window system reports nothing; only an HDR display reports more.
	const float white = SDL_GetFloatProperty(
	    SDL_GetWindowProperties( window ), SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT, 0.0f );
	if ( white > 1.0f && headroom.current > 1.0f )
		headroom.sdrWhiteNits = white * 80.0f;
	return headroom;
}

Sdl3LayerRange Sdl3ReadLayerRange( SDL_Window *window )
{
	(void)window;
	return Sdl3LayerRange();
}

Sdl3DisplayMode Sdl3ReadDisplayMode( SDL_Window *window )
{
	(void)window;
	return Sdl3DisplayMode();
}

#if !defined( SDL3_WAYLAND_COLOR )
// No color-management-v1 client in this product (sdl3_dynamic_range_wayland.cpp).
bool Sdl3PrepareOutputDescription( SDL_Window *, Sdl3OutputDescription * )
{
	return false;
}
bool Sdl3AttachOutputDescription( SDL_Window * )
{
	return false;
}
void Sdl3DetachOutputDescription( SDL_Window * ) {}
bool Sdl3OutputDescriptionChanged( SDL_Window * )
{
	return false;
}
void Sdl3ReleaseOutputDescription( SDL_Window * ) {}
#endif

} // namespace render_vulkan

#endif // !UIKit
