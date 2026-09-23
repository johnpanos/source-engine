//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDL3 window system's render surfaces (RFC 0001 "Window and render
//			interop", roadmap R16). Portable code receives render::IRenderSurface;
//			this header, which exposes SDL types, is the window system's private
//			endpoint and is included only by the SDL3 window provider, SDL3
//			presentation bridges and their native tests.
//
//			Sdl3RenderSurfaces borrows windows; the window's owner creates and
//			destroys the SDL_Window and must call InvalidateWindow before
//			SDL_DestroyWindow, so the attached presentation releases its native
//			objects while the window still exists.
//
//=============================================================================//

#ifndef PLATFORM_SDL3_RENDER_SURFACES_H
#define PLATFORM_SDL3_RENDER_SURFACES_H

#include "render/render_presentation.h"

#include <SDL3/SDL.h>

#include <vector>

namespace platform_sdl3
{

class Sdl3RenderSurface;

class Sdl3RenderSurfaces
{
public:
	Sdl3RenderSurfaces() = default;
	~Sdl3RenderSurfaces();

	Sdl3RenderSurfaces( const Sdl3RenderSurfaces & ) = delete;
	Sdl3RenderSurfaces &operator=( const Sdl3RenderSurfaces & ) = delete;

	// Wraps a live window. The window must outlive InvalidateWindow.
	render::IRenderSurface *Adopt( SDL_Window *window );

	// Maps application lifecycle and window events onto the surfaces: entering
	// the background releases native surfaces, returning restores them, and a
	// destroyed window invalidates its surface.
	void HandleEvent( const SDL_Event &event );

	void ReleaseNativeSurface( render::IRenderSurface &surface );
	void RestoreNativeSurface( render::IRenderSurface &surface );
	void InvalidateWindow( render::IRenderSurface &surface );
	void Destroy( render::IRenderSurface *surface );

	// -- Private endpoint for SDL3 presentation bridges --
	bool Owns( const render::IRenderSurface &surface ) const;
	// The window behind a surface, or nullptr for a foreign, destroyed or
	// currently unavailable surface.
	SDL_Window *NativeWindow( const render::IRenderSurface &surface ) const;

private:
	Sdl3RenderSurface *Find( const render::IRenderSurface &surface ) const;

	std::vector<Sdl3RenderSurface *> m_Surfaces;
};

} // namespace platform_sdl3

#endif // PLATFORM_SDL3_RENDER_SURFACES_H
