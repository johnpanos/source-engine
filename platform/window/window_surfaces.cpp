//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Window-owned render surfaces. See window_surfaces.h.
//
//=============================================================================//

#include "window_surfaces.h"

#include <algorithm>

namespace platform::window
{

using render::IRenderSurfaceListener;
using render::RenderExtent;
using render::RenderSurfaceStatus;

class WindowSurface final : public render::IRenderSurface
{
public:
	WindowSurface( WindowId window, RenderExtent pixels, bool visible )
	    : m_Window( window ), m_Pixels( pixels ), m_Visible( visible )
	{
	}

	RenderExtent GetDrawableExtent() const override
	{
		if ( m_Status != RenderSurfaceStatus::kAvailable || !m_Visible )
			return RenderExtent{};
		return m_Pixels;
	}

	RenderSurfaceStatus GetStatus() const override { return m_Status; }
	uint64_t GetGeneration() const override { return m_Generation; }

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

	WindowId Window() const { return m_Window; }
	bool IsLive() const { return m_Status != RenderSurfaceStatus::kDestroyed; }
	bool IsHeld() const { return m_Listener != nullptr; }

	void SetPixels( RenderExtent pixels ) { m_Pixels = pixels; }
	void SetVisible( bool visible ) { m_Visible = visible; }

	void Release()
	{
		if ( m_Status != RenderSurfaceStatus::kAvailable )
			return;
		NotifyReleasing();
		m_Status = RenderSurfaceStatus::kUnavailable;
	}

	void Restore()
	{
		if ( m_Status != RenderSurfaceStatus::kUnavailable )
			return;
		++m_Generation;
		m_Status = RenderSurfaceStatus::kAvailable;
	}

	void Invalidate()
	{
		if ( m_Status == RenderSurfaceStatus::kDestroyed )
			return;
		if ( m_Status == RenderSurfaceStatus::kAvailable )
			NotifyReleasing();
		m_Status = RenderSurfaceStatus::kDestroyed;
	}

private:
	void NotifyReleasing()
	{
		if ( m_Listener )
			m_Listener->OnNativeSurfaceReleasing();
	}

	WindowId m_Window;
	RenderExtent m_Pixels;
	bool m_Visible;
	RenderSurfaceStatus m_Status = RenderSurfaceStatus::kAvailable;
	uint64_t m_Generation = 0;
	IRenderSurfaceListener *m_Listener = nullptr;
};

WindowSurfaces::WindowSurfaces() = default;

WindowSurfaces::~WindowSurfaces()
{
	InvalidateAll();
}

void WindowSurfaces::Add( WindowId window, RenderExtent pixels, bool visible )
{
	m_Surfaces.push_back( std::make_unique<WindowSurface>( window, pixels, visible ) );
}

render::IRenderSurface *WindowSurfaces::Find( WindowId window ) const
{
	for ( const std::unique_ptr<WindowSurface> &surface : m_Surfaces )
	{
		if ( surface->IsLive() && surface->Window() == window )
			return surface.get();
	}
	return nullptr;
}

void WindowSurfaces::SetPixelExtent( WindowId window, RenderExtent pixels )
{
	if ( WindowSurface *surface = static_cast<WindowSurface *>( Find( window ) ) )
		surface->SetPixels( pixels );
}

void WindowSurfaces::SetVisible( WindowId window, bool visible )
{
	if ( WindowSurface *surface = static_cast<WindowSurface *>( Find( window ) ) )
		surface->SetVisible( visible );
}

void WindowSurfaces::ReleaseAll()
{
	for ( const std::unique_ptr<WindowSurface> &surface : m_Surfaces )
		surface->Release();
}

void WindowSurfaces::RestoreAll()
{
	for ( const std::unique_ptr<WindowSurface> &surface : m_Surfaces )
		surface->Restore();
}

void WindowSurfaces::Invalidate( WindowId window )
{
	if ( WindowSurface *surface = static_cast<WindowSurface *>( Find( window ) ) )
		surface->Invalidate();
	Collect();
}

void WindowSurfaces::Collect()
{
	std::erase_if( m_Surfaces, []( const std::unique_ptr<WindowSurface> &surface )
	    { return !surface->IsLive() && !surface->IsHeld(); } );
}

void WindowSurfaces::InvalidateAll()
{
	for ( const std::unique_ptr<WindowSurface> &surface : m_Surfaces )
		surface->Invalidate();
	Collect();
}

bool WindowSurfaces::Owns( const render::IRenderSurface &surface ) const
{
	return std::any_of( m_Surfaces.begin(), m_Surfaces.end(),
	    [&]( const std::unique_ptr<WindowSurface> &own ) { return own.get() == &surface; } );
}

} // namespace platform::window
