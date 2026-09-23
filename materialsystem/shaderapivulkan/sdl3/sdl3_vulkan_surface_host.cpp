//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: IVulkanSurfaceHost over an SDL3 window. See sdl3_vulkan_surface_host.h.
//
//===========================================================================//

#include "sdl3_vulkan_surface_host.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace render_vulkan
{

namespace
{

class Sdl3WindowSurfaceHost : public IVulkanSurfaceHost
{
public:
	explicit Sdl3WindowSurfaceHost( SDL_Window *window ) : m_Window( window ) {}

	bool GetInstanceExtensions(
	    std::vector<const char *> *outExtensions, std::string *outError ) const override
	{
		Uint32 count = 0;
		char const *const *names = SDL_Vulkan_GetInstanceExtensions( &count );
		if ( !names )
		{
			if ( outError )
				*outError =
				    std::string( "SDL_Vulkan_GetInstanceExtensions failed: " ) + SDL_GetError();
			return false;
		}
		outExtensions->assign( names, names + count );
		return true;
	}

	bool CreateSurface(
	    VkInstance instance, VkSurfaceKHR *outSurface, std::string *outError ) override
	{
		if ( !SDL_Vulkan_CreateSurface( m_Window, instance, nullptr, outSurface ) )
		{
			if ( outError )
				*outError = std::string( "SDL_Vulkan_CreateSurface failed: " ) + SDL_GetError();
			return false;
		}
		return true;
	}

	void DestroySurface( VkInstance instance, VkSurfaceKHR surface ) override
	{
		SDL_Vulkan_DestroySurface( instance, surface, nullptr );
	}

	void GetDrawableSize( int *outWidth, int *outHeight ) const override
	{
		SDL_GetWindowSizeInPixels( m_Window, outWidth, outHeight );
	}

	bool IsNativeSurfaceAvailable() const override { return CurrentNativeWindow() != nullptr; }

	uint64_t GetNativeSurfaceGeneration() const override
	{
		const void *native = CurrentNativeWindow();
		if ( native != m_LastNative )
		{
			m_LastNative = native;
			++m_Generation;
		}
		return m_Generation;
	}

private:
	// The platform surface the window currently presents to. Android replaces its
	// ANativeWindow across backgrounding and some display changes; elsewhere a
	// window keeps one surface for its life, reported as the window itself.
	const void *CurrentNativeWindow() const
	{
#if defined( __ANDROID__ )
		return SDL_GetPointerProperty(
		    SDL_GetWindowProperties( m_Window ), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr );
#else
		return m_Window;
#endif
	}

	SDL_Window *m_Window;
	mutable const void *m_LastNative = nullptr;
	mutable uint64_t m_Generation = 0;
};

} // namespace

std::unique_ptr<IVulkanSurfaceHost> MakeSdl3LegacySurfaceHost(
    void *legacyWindowRef, std::string *outError )
{
	if ( !legacyWindowRef )
	{
		if ( outError )
			*outError = "no engine window to present to";
		return nullptr;
	}
	return std::unique_ptr<IVulkanSurfaceHost>(
	    new Sdl3WindowSurfaceHost( static_cast<SDL_Window *>( legacyWindowRef ) ) );
}

} // namespace render_vulkan
