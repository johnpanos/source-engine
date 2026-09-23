//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Window-owned render surfaces for window providers (RFC 0001 rank 7,
//			roadmap R14). Implements R16's render::IRenderSurface without native
//			types; the provider reports sizes and lifecycle, and this registry
//			applies the render.presentation.v1 surface rules:
//
//			  - the drawable extent is zero while a window is hidden or minimized;
//			  - entering the background releases every native surface (the attached
//			    presentation is told first) and returning restores them with a new
//			    generation, the same mapping as platform/sdl3/render_surface;
//			  - destroying a window tells the attached presentation, then reports
//			    kDestroyed; the surface object lives until it has no presentation,
//			    and Collect reclaims it after that.
//
//			Single-sequence: called only on the window provider's sequence.
//
//=============================================================================//

#ifndef PLATFORM_WINDOW_WINDOW_SURFACES_H
#define PLATFORM_WINDOW_WINDOW_SURFACES_H

#include "platform/window/window_events.h"
#include "render/render_presentation.h"

#include <memory>
#include <vector>

namespace platform::window
{

class WindowSurface;

class WindowSurfaces
{
public:
	WindowSurfaces();
	~WindowSurfaces();

	WindowSurfaces( const WindowSurfaces & ) = delete;
	WindowSurfaces &operator=( const WindowSurfaces & ) = delete;

	// Registers the surface of a new window with its drawable extent in pixels.
	void Add( WindowId window, render::RenderExtent pixels, bool visible );

	// Live-window surface, or null for an unknown or destroyed window.
	render::IRenderSurface *Find( WindowId window ) const;

	void SetPixelExtent( WindowId window, render::RenderExtent pixels );
	void SetVisible( WindowId window, bool visible ); // hidden or minimized when false

	void ReleaseAll(); // entering the background
	void RestoreAll(); // returning to the foreground

	// Window destruction: tells the attached presentation, then marks the
	// surface kDestroyed. The object stays valid while a presentation is attached.
	void Invalidate( WindowId window );

	// Frees destroyed surfaces that no presentation holds any longer.
	void Collect();

	// Invalidates every window (provider shutdown). Surfaces a presentation
	// still holds stay allocated until this registry is destroyed.
	void InvalidateAll();

	// True for a surface this registry created, including destroyed ones.
	bool Owns( const render::IRenderSurface &surface ) const;
	std::size_t AllocatedCount() const { return m_Surfaces.size(); }

private:
	std::vector<std::unique_ptr<WindowSurface>> m_Surfaces;
};

} // namespace platform::window

#endif // PLATFORM_WINDOW_WINDOW_SURFACES_H
