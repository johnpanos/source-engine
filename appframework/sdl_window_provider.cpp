//========= Copyright Valve Corporation, All rights reserved. ============//
#include "appframework/window_provider.h"
#include "appframework/ilaunchermgr.h"
#include "tier0/dbg.h"
#include <SDL.h>
#include <SDL_version.h>

extern void *CreateSDLMgr();

namespace
{
ILauncherMgr *CreateWindowProvider()
{
	return static_cast<ILauncherMgr *>( CreateSDLMgr() );
}
} // namespace

const WindowProviderDescriptor *WindowProvider_Describe()
{
#ifdef USE_SDL3
	static const WindowProviderDescriptor provider = { "sdl3", CreateWindowProvider };
#else
	static const WindowProviderDescriptor provider = { "sdl2", CreateWindowProvider };
#endif
	return &provider;
}

void WindowProvider_ReportVersion()
{
#ifdef USE_SDL3
	const int version = SDL_GetVersion();
	Msg( "SDL version: %d.%d.%d rev: %s\n", SDL_VERSIONNUM_MAJOR( version ),
	    SDL_VERSIONNUM_MINOR( version ), SDL_VERSIONNUM_MICRO( version ), SDL_GetRevision() );
#else
	SDL_version version;
	SDL_GetVersion( &version );
	Msg( "SDL version: %d.%d.%d rev: %s\n", version.major, version.minor, version.patch,
	    SDL_GetRevision() );
#endif
}

void WindowProvider_ShowError( const char *title, const char *message )
{
	SDL_ShowSimpleMessageBox( 0, title, message, GetAssertDialogParent() );
}

void WindowProvider_Prepare()
{
#if ( defined( LINUX ) || defined( PLATFORM_BSD ) ) && defined( TOGLES ) && !defined( ANDROID )
	SDL_SetHint( SDL_HINT_VIDEO_X11_FORCE_EGL, "1" );
#endif
}
