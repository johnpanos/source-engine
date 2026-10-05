//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The macOS half of the Apple app root (apple_app.h).
//
// Content lives in the app's Application Support directory,
// ~/Library/Application Support/<bundle id>/, the per-app location that is
// the same with or without the App Sandbox. The app is a desktop window: its
// size, mode and the desktop GameUI come from the player's configuration, as
// on Linux.
//
//=============================================================================//

#include <CoreFoundation/CoreFoundation.h>
#include <SDL3/SDL.h>

#include <stdio.h>

#include "apple_app.h"

namespace appleapp
{

void ConfigureSdl()
{
	// SDL's macOS defaults apply.
}

const char *ContentDirectory()
{
	char bundleId[256];
	CFStringRef identifier = CFBundleGetIdentifier( CFBundleGetMainBundle() );
	if ( !identifier ||
	     !CFStringGetCString( identifier, bundleId, sizeof( bundleId ), kCFStringEncodingUTF8 ) )
	{
		SDL_SetError( "the app bundle has no CFBundleIdentifier" );
		return NULL;
	}
	// Created when missing, so the player has a folder to copy the content into.
	static char *s_Directory = SDL_GetPrefPath( "", bundleId );
	return s_Directory;
}

void ReportMissingContent( const char *contentDir )
{
	char reason[1024];
	snprintf( reason, sizeof( reason ),
	    "The game content is missing. Copy the platform, %s and hl2 folders into:\n\n%s",
	    APPLE_DEFAULT_GAME, contentDir );
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Game content missing", reason, NULL );
}

void PrepareContent( const char *, const char * )
{
}

bool BindPlatformServices( const char * )
{
	// No platform achievement service yet (Game Center's presentation is UIKit
	// only, platform/apple/game_center_achievements.mm); the game modules keep
	// their file record store, as on the Linux desktop.
	return true;
}

int AppendLaunchArguments( char **, int argc, int, const char * )
{
	return argc;
}

int AppendDefaultArguments( char **, int argc, int )
{
	return argc;
}

} // namespace appleapp
