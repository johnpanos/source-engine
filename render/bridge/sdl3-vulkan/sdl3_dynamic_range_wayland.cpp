//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDL3-Vulkan pair's output image description on Wayland
//			(color-management-v1): a window's surface takes the output's own
//			preferred image description, so its HDR10 frames need no color
//			mapping and a compositor may scan them out directly (mutter
//			composites any surface whose color state differs from the
//			output's, luminances included).
//
//			The swapchain is then created with VK_COLOR_SPACE_PASS_THROUGH_EXT,
//			for which Mesa's WSI declares nothing, leaving the surface's
//			color-management object to this file. A swapchain in any other
//			color space has the WSI declare its own: Detach first, since a
//			surface takes one color-management object (surface_exists).
//
//			All requests and events go through a private event queue on SDL's
//			wl_display; SDL's own event loop reads the socket.
//
//=============================================================================//

#include "sdl3_dynamic_range.h"

#include "wayland/color-management-v1-client-protocol.h"

#include <SDL3/SDL.h>
#include <wayland-client.h>

#include <cstdint>
#include <cstring>
#include <mutex>
#include <unordered_map>

#include <unistd.h>

namespace render_vulkan
{

namespace
{

struct Description
{
	wp_image_description_v1 *object = nullptr;
	bool ready = false;
	bool failed = false;
};

struct Info
{
	bool done = false;
	uint32_t primaries = 0;
	uint32_t tf = 0;
	uint32_t minLum = 0, maxLum = 0, refLum = 0;
};

struct OutputColor
{
	wl_display *display = nullptr;
	wl_surface *surface = nullptr;
	wl_event_queue *queue = nullptr;
	wl_display *wrapper = nullptr; // the display on 'queue'
	wl_registry *registry = nullptr;
	wp_color_manager_v1 *manager = nullptr;
	uint32_t managerVersion = 0;
	wp_color_management_surface_feedback_v1 *feedback = nullptr;
	wp_color_management_surface_v1 *colorSurface = nullptr;
	Description preferred;
	Info info;
	bool changed = false;

	~OutputColor()
	{
		if ( colorSurface )
			wp_color_management_surface_v1_destroy( colorSurface );
		if ( preferred.object )
			wp_image_description_v1_destroy( preferred.object );
		if ( feedback )
			wp_color_management_surface_feedback_v1_destroy( feedback );
		if ( manager )
			wp_color_manager_v1_destroy( manager );
		if ( registry )
			wl_registry_destroy( registry );
		if ( wrapper )
			wl_proxy_wrapper_destroy( wrapper );
		if ( display )
			wl_display_flush( display );
		if ( queue )
			wl_event_queue_destroy( queue );
	}
};

std::mutex g_Lock;
std::unordered_map<SDL_Window *, OutputColor *> g_Windows;

void OnGlobal( void *data, wl_registry *registry, uint32_t name, const char *interface,
    uint32_t version )
{
	OutputColor *c = static_cast<OutputColor *>( data );
	if ( !c->manager && std::strcmp( interface, wp_color_manager_v1_interface.name ) == 0 )
	{
		c->managerVersion = version < 2 ? version : 2;
		c->manager = static_cast<wp_color_manager_v1 *>( wl_registry_bind(
		    registry, name, &wp_color_manager_v1_interface, c->managerVersion ) );
	}
}
void OnGlobalRemove( void *, wl_registry *, uint32_t ) {}
const wl_registry_listener kRegistry = { OnGlobal, OnGlobalRemove };

// The manager's capability events: only the image description path is used.
void OnIntent( void *, wp_color_manager_v1 *, uint32_t ) {}
void OnFeature( void *, wp_color_manager_v1 *, uint32_t ) {}
void OnTf( void *, wp_color_manager_v1 *, uint32_t ) {}
void OnPrimaries( void *, wp_color_manager_v1 *, uint32_t ) {}
void OnManagerDone( void *, wp_color_manager_v1 * ) {}
const wp_color_manager_v1_listener kManager = { OnIntent, OnFeature, OnTf, OnPrimaries,
	OnManagerDone };

void OnPreferredChanged( void *data, wp_color_management_surface_feedback_v1 *, uint32_t )
{
	static_cast<OutputColor *>( data )->changed = true;
}
void OnPreferredChanged2( void *data, wp_color_management_surface_feedback_v1 *, uint32_t, uint32_t )
{
	static_cast<OutputColor *>( data )->changed = true;
}
const wp_color_management_surface_feedback_v1_listener kFeedback = { OnPreferredChanged,
	OnPreferredChanged2 };

void OnFailed( void *data, wp_image_description_v1 *, uint32_t, const char * )
{
	static_cast<Description *>( data )->failed = true;
}
void OnReady( void *data, wp_image_description_v1 *, uint32_t )
{
	static_cast<Description *>( data )->ready = true;
}
void OnReady2( void *data, wp_image_description_v1 *, uint32_t, uint32_t )
{
	static_cast<Description *>( data )->ready = true;
}
const wp_image_description_v1_listener kDescription = { OnFailed, OnReady, OnReady2 };

using InfoObj = wp_image_description_info_v1;
// done is a destructor event: the client destroys the proxy.
void OnInfoDone( void *data, InfoObj *info )
{
	static_cast<Info *>( data )->done = true;
	wl_proxy_destroy( reinterpret_cast<wl_proxy *>( info ) );
}
void OnIcc( void *, InfoObj *, int32_t fd, uint32_t ) { close( fd ); }
void OnPrim( void *, InfoObj *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t,
    int32_t )
{
}
void OnPrimNamed( void *data, InfoObj *, uint32_t primaries )
{
	static_cast<Info *>( data )->primaries = primaries;
}
void OnTfPower( void *, InfoObj *, uint32_t ) {}
void OnTfNamed( void *data, InfoObj *, uint32_t tf ) { static_cast<Info *>( data )->tf = tf; }
void OnLum( void *data, InfoObj *, uint32_t minLum, uint32_t maxLum, uint32_t refLum )
{
	Info *info = static_cast<Info *>( data );
	info->minLum = minLum;
	info->maxLum = maxLum;
	info->refLum = refLum;
}
void OnTargetLum( void *, InfoObj *, uint32_t, uint32_t ) {}
void OnTargetValue( void *, InfoObj *, uint32_t ) {}
const wp_image_description_info_v1_listener kInfo = { OnInfoDone, OnIcc, OnPrim, OnPrimNamed,
	OnTfPower, OnTfNamed, OnLum, OnPrim, OnTargetLum, OnTargetValue, OnTargetValue };

// Bounded: a compositor that never answers does not hang a frame.
template <typename Done> bool Roundtrip( OutputColor &c, Done done )
{
	for ( int i = 0; i < 8 && !done(); ++i )
		if ( wl_display_roundtrip_queue( c.display, c.queue ) < 0 )
			return false;
	return done();
}

// Fetches the output's preferred description and what it says.
bool FetchPreferred( OutputColor &c )
{
	if ( c.preferred.object )
		wp_image_description_v1_destroy( c.preferred.object );
	c.preferred = Description();
	c.info = Info();
	c.preferred.object = wp_color_management_surface_feedback_v1_get_preferred( c.feedback );
	wp_image_description_v1_add_listener( c.preferred.object, &kDescription, &c.preferred );
	if ( !Roundtrip( c, [&] { return c.preferred.ready || c.preferred.failed; } ) ||
	     c.preferred.failed )
		return false;
	InfoObj *info = wp_image_description_v1_get_information( c.preferred.object );
	wp_image_description_info_v1_add_listener( info, &kInfo, &c.info );
	const bool done = Roundtrip( c, [&] { return c.info.done; } );
	if ( !done )
		wl_proxy_destroy( reinterpret_cast<wl_proxy *>( info ) );
	return done;
}

OutputColor *Open( SDL_Window *window )
{
	auto found = g_Windows.find( window );
	if ( found != g_Windows.end() )
		return found->second;
	const SDL_PropertiesID props = SDL_GetWindowProperties( window );
	wl_display *display = static_cast<wl_display *>(
	    SDL_GetPointerProperty( props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr ) );
	wl_surface *surface = static_cast<wl_surface *>(
	    SDL_GetPointerProperty( props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr ) );
	if ( !display || !surface )
		return nullptr; // not a Wayland window
	OutputColor *c = new OutputColor;
	c->display = display;
	c->surface = surface;
	c->queue = wl_display_create_queue( display );
	c->wrapper = static_cast<wl_display *>( wl_proxy_create_wrapper( display ) );
	wl_proxy_set_queue( reinterpret_cast<wl_proxy *>( c->wrapper ), c->queue );
	c->registry = wl_display_get_registry( c->wrapper );
	wl_registry_add_listener( c->registry, &kRegistry, c );
	if ( !Roundtrip( *c, [&] { return c->manager != nullptr; } ) )
	{
		delete c;
		g_Windows[window] = nullptr;
		return nullptr;
	}
	wp_color_manager_v1_add_listener( c->manager, &kManager, c );
	c->feedback = wp_color_manager_v1_get_surface_feedback( c->manager, surface );
	wp_color_management_surface_feedback_v1_add_listener( c->feedback, &kFeedback, c );
	g_Windows[window] = c;
	return c;
}

bool IsHdr10( const Info &info )
{
	return info.primaries == WP_COLOR_MANAGER_V1_PRIMARIES_BT2020 &&
	       info.tf == WP_COLOR_MANAGER_V1_TRANSFER_FUNCTION_ST2084_PQ;
}

} // namespace

bool Sdl3PrepareOutputDescription( SDL_Window *window, Sdl3OutputDescription *out )
{
	std::lock_guard<std::mutex> lock( g_Lock );
	OutputColor *c = window ? Open( window ) : nullptr;
	if ( !c || !FetchPreferred( *c ) )
		return false;
	c->changed = false;
	if ( out )
	{
		out->hdr10 = IsHdr10( c->info );
		out->minNits = c->info.minLum / 10000.0f;
		out->maxNits = float( c->info.maxLum );
		out->referenceNits = float( c->info.refLum );
	}
	return IsHdr10( c->info );
}

bool Sdl3AttachOutputDescription( SDL_Window *window )
{
	std::lock_guard<std::mutex> lock( g_Lock );
	auto found = g_Windows.find( window );
	OutputColor *c = found != g_Windows.end() ? found->second : nullptr;
	if ( !c || !c->preferred.ready || !IsHdr10( c->info ) )
		return false;
	if ( !c->colorSurface )
		c->colorSurface = wp_color_manager_v1_get_surface( c->manager, c->surface );
	// Double-buffered: the WSI's next present commits it with the frame.
	wp_color_management_surface_v1_set_image_description( c->colorSurface, c->preferred.object,
	    WP_COLOR_MANAGER_V1_RENDER_INTENT_PERCEPTUAL );
	return wl_display_flush( c->display ) >= 0;
}

void Sdl3DetachOutputDescription( SDL_Window *window )
{
	std::lock_guard<std::mutex> lock( g_Lock );
	auto found = g_Windows.find( window );
	OutputColor *c = found != g_Windows.end() ? found->second : nullptr;
	if ( !c || !c->colorSurface )
		return;
	wp_color_management_surface_v1_destroy( c->colorSurface );
	c->colorSurface = nullptr;
	wl_display_flush( c->display );
}

bool Sdl3OutputDescriptionChanged( SDL_Window *window )
{
	std::lock_guard<std::mutex> lock( g_Lock );
	auto found = g_Windows.find( window );
	OutputColor *c = found != g_Windows.end() ? found->second : nullptr;
	if ( !c )
		return false;
	wl_display_dispatch_queue_pending( c->display, c->queue );
	return c->changed;
}

void Sdl3ReleaseOutputDescription( SDL_Window *window )
{
	std::lock_guard<std::mutex> lock( g_Lock );
	auto found = g_Windows.find( window );
	if ( found == g_Windows.end() )
		return;
	delete found->second;
	g_Windows.erase( found );
}

} // namespace render_vulkan
