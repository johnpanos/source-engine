//========= Copyright Valve Corporation, All rights reserved. ============//
#include "appframework/window_provider.h"
#include "appframework/ilaunchermgr.h"
#include "tier0/dbg.h"
#include <SDL3/SDL.h>

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
	static const WindowProviderDescriptor provider = { "sdl3", CreateWindowProvider };
	return &provider;
}

void WindowProvider_ReportVersion()
{
	const int version = SDL_GetVersion();
	Msg( "SDL version: %d.%d.%d rev: %s\n", SDL_VERSIONNUM_MAJOR( version ),
	    SDL_VERSIONNUM_MINOR( version ), SDL_VERSIONNUM_MICRO( version ), SDL_GetRevision() );
}

void WindowProvider_ShowError( const char *title, const char *message )
{
	SDL_ShowSimpleMessageBox( 0, title, message, GetAssertDialogParent() );
}

void WindowProvider_Prepare()
{
}
