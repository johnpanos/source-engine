//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The iOS app entry. SDL3 owns main() here (SDL_main.h): it starts UIKit and
// then calls this main() on the app's main thread. Every first-party module is
// linked into the app (static_composition.cpp); nothing is loaded at run time.
//
// Content (platform/, portal/, hl2/ and an optional commandline.txt) lives in
// the app's Documents container, which the Mac copies it into
// (xcrun devicectl ... --domain-type appDataContainer) and which the Files app
// can reach (UIFileSharingEnabled).
//
//=============================================================================//

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mobile_app_root.h"
#include "static_composition.h"
#include "tier0/platform.h"
#include "tier0/threadtools.h"

extern "C" int LauncherMain( int argc, char **argv );

namespace
{

// A directory path without its trailing separator.
void CopyDirectory( char *out, size_t size, const char *path )
{
	snprintf( out, size, "%s", path );
	const size_t length = strlen( out );
	if ( length > 1 && out[length - 1] == '/' )
		out[length - 1] = '\0';
}

} // namespace

int main( int, char ** )
{
	DeclareCurrentThreadIsMainThread();

	// Every orientation the device and the user's rotation lock allow, as on
	// Android; Info.plist declares the same set. The back buffer follows the
	// drawable (mat_windowed_fullscreen, engine/sys_getmodes.cpp).
	SDL_SetHint(
	    SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown" );
	// Fingers reach the game only as touch events: the client's touch controls
	// (game/client/touch.cpp) and VGUI's finger handling, not synthesized clicks.
	SDL_SetHint( SDL_HINT_TOUCH_MOUSE_EVENTS, "0" );
	// The on-screen keyboard has no hide key on iPhone and there is no back
	// button: Return (which submits a console or chat line) also hides it, and
	// a text field hides it when it loses focus or a tap lands outside it.
	SDL_SetHint( SDL_HINT_RETURN_KEY_HIDES_IME, "1" );

	const char *bundle = SDL_GetBasePath();
	const char *documents = SDL_GetUserFolder( SDL_FOLDER_DOCUMENTS );
	if ( !bundle || !documents )
	{
		SDL_Log( "Source: app storage is unavailable: %s", SDL_GetError() );
		return 1;
	}
	static char bundleDir[PATH_MAX], contentDir[PATH_MAX];
	CopyDirectory( bundleDir, sizeof( bundleDir ), bundle );
	CopyDirectory( contentDir, sizeof( contentDir ), documents );

	// The filesystem and launcher read these on the app-container platforms
	// (public/filesystem_init.cpp, launcher/launcher.cpp,
	// vgui2/vgui_surfacelib/linuxfont.cpp).
	setenv( "APP_LIB_PATH", bundleDir, 1 );
	setenv( "VALVE_GAME_PATH", contentDir, 1 );
	setenv( "APP_DATA_PATH", contentDir, 1 );
	// Game directories (-game) are relative to the content root.
	if ( chdir( contentDir ) != 0 )
	{
		SDL_Log( "Source: cannot enter the content directory %s", contentDir );
		return 1;
	}

	// The app's own UI art ships in the bundle's touch/ directory.
	static char assetRoot[PATH_MAX];
	snprintf( assetRoot, sizeof( assetRoot ), "%s/", bundleDir );
	mobileapp::InstallTouchIcons( assetRoot, IOS_DEFAULT_GAME );

	if ( !StaticComposition_BindGame() )
	{
		SDL_Log( "Source: failed to bind the linked game modules" );
		return 1;
	}

	static char program[PATH_MAX];
	snprintf( program, sizeof( program ), "%s/hl2_launcher", bundleDir );
	static char *argv[mobileapp::kMaxArgs];
	int argc = 0;
	argv[argc++] = program;
	argv[argc++] = const_cast<char *>( "-game" );
	argv[argc++] = const_cast<char *>( IOS_DEFAULT_GAME );
	argv[argc++] = const_cast<char *>( "-renderer" );
	argv[argc++] = const_cast<char *>( "native-vulkan" );
	argv[argc++] = const_cast<char *>( "-fullscreen" );
	// The system owns the surface size (rotation, split view).
	argv[argc++] = const_cast<char *>( "+mat_windowed_fullscreen" );
	argv[argc++] = const_cast<char *>( "1" );
	argv[argc++] = const_cast<char *>( "-nosteam" );
	argv[argc++] = const_cast<char *>( "-insecure" );
	argv[argc++] = const_cast<char *>( "-nouserclip" );
	// The material system renders on the main thread, as on Android, until
	// its render thread is measured on the mobile profiles; commandline.txt
	// may ask for +mat_queue_mode 2.
	argv[argc++] = const_cast<char *>( "+mat_queue_mode" );
	argv[argc++] = const_cast<char *>( "0" );

	static char argumentsPath[PATH_MAX];
	static char argumentsStorage[4096];
	snprintf( argumentsPath, sizeof( argumentsPath ), "%s/commandline.txt", contentDir );
	argc = mobileapp::AppendArgumentsFile(
	    argumentsPath, argv, argc, argumentsStorage, sizeof( argumentsStorage ) );
	argv[argc] = NULL;

	SDL_Log( "Source: bundle=%s content=%s", bundleDir, contentDir );
	return LauncherMain( argc, argv );
}
