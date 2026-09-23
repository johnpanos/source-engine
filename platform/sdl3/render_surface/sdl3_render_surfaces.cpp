//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDL3 render surfaces. See sdl3_render_surfaces.h.
//
//=============================================================================//

#include "sdl3_render_surfaces.h"

namespace platform_sdl3
{

using render::IRenderSurface;
using render::IRenderSurfaceListener;
using render::RenderExtent;
using render::RenderSurfaceStatus;

class Sdl3RenderSurface : public IRenderSurface
{
public:
	explicit Sdl3RenderSurface( SDL_Window *window ) : m_Window( window ) {}

	RenderExtent GetDrawableExtent() const override
	{
		RenderExtent extent;
		if ( GetStatus() != RenderSurfaceStatus::kAvailable )
			return extent;
		if ( SDL_GetWindowFlags( m_Window ) & ( SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED ) )
			return extent;
		int width = 0, height = 0;
		if ( !SDL_GetWindowSizeInPixels( m_Window, &width, &height ) || width <= 0 || height <= 0 )
			return extent;
		extent.width = static_cast<uint32_t>( width );
		extent.height = static_cast<uint32_t>( height );
		return extent;
	}

	RenderSurfaceStatus GetStatus() const override
	{
		if ( m_Status != RenderSurfaceStatus::kAvailable )
			return m_Status;
		return CurrentNative() ? RenderSurfaceStatus::kAvailable : RenderSurfaceStatus::kUnavailable;
	}

	uint64_t GetGeneration() const override
	{
		// A platform that swaps the native window underneath SDL (Android) also
		// advances the generation, even without an explicit release.
		if ( m_Status == RenderSurfaceStatus::kAvailable )
		{
			const void *native = CurrentNative();
			if ( native != m_LastNative )
			{
				m_LastNative = native;
				++m_Generation;
			}
		}
		return m_Generation;
	}

	bool AttachListener( IRenderSurfaceListener &listener ) override
	{
		if ( m_Listener )
			return false;
		m_Listener = &listener;
		return true;
	}

	void DetachListener( IRenderSurfaceListener &listener ) override
	{
		if ( m_Listener == &listener )
			m_Listener = nullptr;
	}

	void Release()
	{
		if ( m_Status != RenderSurfaceStatus::kAvailable )
			return;
		if ( m_Listener )
			m_Listener->OnNativeSurfaceReleasing();
		m_Status = RenderSurfaceStatus::kUnavailable;
	}

	void Restore()
	{
		if ( m_Status != RenderSurfaceStatus::kUnavailable )
			return;
		++m_Generation;
		m_LastNative = CurrentNative();
		m_Status = RenderSurfaceStatus::kAvailable;
	}

	void Invalidate()
	{
		if ( m_Status == RenderSurfaceStatus::kDestroyed )
			return;
		if ( m_Status == RenderSurfaceStatus::kAvailable && m_Listener )
			m_Listener->OnNativeSurfaceReleasing();
		m_Status = RenderSurfaceStatus::kDestroyed;
		m_Window = nullptr;
	}

	SDL_Window *Window() const { return m_Window; }

private:
	// The platform surface the window presents to. Android replaces its
	// ANativeWindow across backgrounding; elsewhere it is the window itself.
	const void *CurrentNative() const
	{
		if ( !m_Window )
			return nullptr;
#if defined( __ANDROID__ )
		return SDL_GetPointerProperty( SDL_GetWindowProperties( m_Window ),
			SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr );
#else
		return m_Window;
#endif
	}

	SDL_Window *m_Window;
	RenderSurfaceStatus m_Status = RenderSurfaceStatus::kAvailable;
	mutable const void *m_LastNative = nullptr;
	mutable uint64_t m_Generation = 0;
	IRenderSurfaceListener *m_Listener = nullptr;
};

Sdl3RenderSurfaces::~Sdl3RenderSurfaces()
{
	for ( Sdl3RenderSurface *s : m_Surfaces )
		delete s;
}

IRenderSurface *Sdl3RenderSurfaces::Adopt( SDL_Window *window )
{
	if ( !window )
		return nullptr;
	Sdl3RenderSurface *s = new Sdl3RenderSurface( window );
	s->GetGeneration(); // bind the generation to the current native surface
	m_Surfaces.push_back( s );
	return s;
}

Sdl3RenderSurface *Sdl3RenderSurfaces::Find( const IRenderSurface &surface ) const
{
	for ( Sdl3RenderSurface *s : m_Surfaces )
		if ( s == &surface )
			return s;
	return nullptr;
}

void Sdl3RenderSurfaces::HandleEvent( const SDL_Event &event )
{
	switch ( event.type )
	{
	case SDL_EVENT_DID_ENTER_BACKGROUND:
		for ( Sdl3RenderSurface *s : m_Surfaces )
			s->Release();
		break;
	case SDL_EVENT_WILL_ENTER_FOREGROUND:
		for ( Sdl3RenderSurface *s : m_Surfaces )
			s->Restore();
		break;
	case SDL_EVENT_WINDOW_DESTROYED:
		for ( Sdl3RenderSurface *s : m_Surfaces )
			if ( s->Window() && SDL_GetWindowID( s->Window() ) == event.window.windowID )
				s->Invalidate();
		break;
	default:
		break;
	}
}

void Sdl3RenderSurfaces::ReleaseNativeSurface( IRenderSurface &surface )
{
	if ( Sdl3RenderSurface *s = Find( surface ) )
		s->Release();
}

void Sdl3RenderSurfaces::RestoreNativeSurface( IRenderSurface &surface )
{
	if ( Sdl3RenderSurface *s = Find( surface ) )
		s->Restore();
}

void Sdl3RenderSurfaces::InvalidateWindow( IRenderSurface &surface )
{
	if ( Sdl3RenderSurface *s = Find( surface ) )
		s->Invalidate();
}

void Sdl3RenderSurfaces::Destroy( IRenderSurface *surface )
{
	for ( size_t i = 0; i < m_Surfaces.size(); ++i )
	{
		if ( m_Surfaces[i] == surface )
		{
			m_Surfaces[i]->Invalidate();
			delete m_Surfaces[i];
			m_Surfaces.erase( m_Surfaces.begin() + static_cast<std::ptrdiff_t>( i ) );
			return;
		}
	}
}

bool Sdl3RenderSurfaces::Owns( const IRenderSurface &surface ) const
{
	return Find( surface ) != nullptr;
}

SDL_Window *Sdl3RenderSurfaces::NativeWindow( const IRenderSurface &surface ) const
{
	const Sdl3RenderSurface *s = Find( surface );
	if ( !s || s->GetStatus() != RenderSurfaceStatus::kAvailable )
		return nullptr;
	return s->Window();
}

} // namespace platform_sdl3
