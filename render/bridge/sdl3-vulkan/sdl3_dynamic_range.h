//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The window-system half of render.presentation.v1's dynamic range for
//          the SDL3-Vulkan bridge. The bridge chooses the swapchain format and
//          color space; these functions do what the platform needs beyond
//          that:
//          - On Apple, MoltenVK sets the Metal layer's color space but enables
//            EDR only on macOS. The layer's dynamic range is set here, and the
//            screen's headroom is read from UIKit. On tvOS the extended range
//            also asks for the display's HDR mode (AVDisplayManager).
//          - Elsewhere, SDL's window HDR properties report the headroom.
//          Private to render.bridge.sdl3-vulkan. Implemented in
//          sdl3_dynamic_range.cpp and sdl3_dynamic_range_apple.mm.
//
//===========================================================================//

#ifndef RENDER_BRIDGE_SDL3_DYNAMIC_RANGE_H
#define RENDER_BRIDGE_SDL3_DYNAMIC_RANGE_H

struct SDL_Window;

namespace render_vulkan
{

// The display's headroom in multiples of SDR reference white.
struct Sdl3DisplayHeadroom
{
	float current = 1.0f;
	float potential = 1.0f;
	// The system's SDR (paper) white in cd/m^2; 0 where it does not say.
	float sdrWhiteNits = 0.0f;
};

// Whether the platform can show extended-linear output in 'window' at all
// (the API exists and the display has headroom above 1). The surface's format
// list decides the rest.
bool Sdl3CanShowExtendedRange( SDL_Window *window );

// Sets the window's presentation layer to extended range, or back to
// standard. Call after the swapchain is (re)built. False when the platform
// refused. On tvOS the extended range also asks tvOS to switch the display
// into HDR (HDR10 display criteria), and the standard range withdraws that.
bool Sdl3SetExtendedRange( SDL_Window *window, bool extended );

// The display's headroom now. Never blocks: off the main thread on Apple it
// returns the value last read there and schedules a fresh read.
Sdl3DisplayHeadroom Sdl3ReadHeadroom( SDL_Window *window );

// Native test endpoint: what the platform layer reports. 'known' is false
// where the platform has no layer to inspect.
struct Sdl3LayerRange
{
	bool known = false;
	bool extended = false;                 // the layer shows content above SDR white
	bool extendedLinearColorspace = false; // the layer's color space is extended linear sRGB
};
Sdl3LayerRange Sdl3ReadLayerRange( SDL_Window *window );

// Native test endpoint: tvOS's display mode request for the window. 'known'
// is false where there is none (not tvOS, or off the main thread).
struct Sdl3DisplayMode
{
	bool known = false;
	bool matchingEnabled = false; // the user's "Match Dynamic Range" setting is on
	bool switching = false;       // tvOS is switching the display's mode now
	bool askedForHdr = false;     // the window's criteria ask for HDR
	// What the connected display can play (AVPlayer.availableHDRModes): the
	// bits hlg 1, hdr10 2, dolbyVision 4. Zero on a display without HDR.
	unsigned hdrModes = 0;
	bool eligibleForHdr = false; // AVPlayer.eligibleForHDRPlayback
};
Sdl3DisplayMode Sdl3ReadDisplayMode( SDL_Window *window );

// The output's own image description on Wayland (color-management-v1;
// sdl3_dynamic_range_wayland.cpp). A surface that declares it needs no color
// mapping, which lets a compositor scan HDR10 frames out directly. Prepare
// fetches the output's preferred description (true when it is BT.2020/PQ);
// Attach sets it on the surface for a VK_COLOR_SPACE_PASS_THROUGH_EXT
// swapchain (applied by the next present); Detach releases it before a
// swapchain whose WSI declares its own color space; Changed reports the
// compositor's preferred_changed, after which the caller prepares again.
// False and no-ops on other platforms and window systems.
struct Sdl3OutputDescription
{
	bool hdr10 = false;
	float minNits = 0.0f;
	float maxNits = 0.0f;
	float referenceNits = 0.0f;
};
bool Sdl3PrepareOutputDescription( SDL_Window *window, Sdl3OutputDescription *out );
bool Sdl3AttachOutputDescription( SDL_Window *window );
void Sdl3DetachOutputDescription( SDL_Window *window );
bool Sdl3OutputDescriptionChanged( SDL_Window *window );
void Sdl3ReleaseOutputDescription( SDL_Window *window );

} // namespace render_vulkan

#endif // RENDER_BRIDGE_SDL3_DYNAMIC_RANGE_H
