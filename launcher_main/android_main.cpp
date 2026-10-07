//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Android application root for the SDL3/native Vulkan client.
//          SDLActivity loads libSDL3.so and this libmain.so from the APK's
//          native library directory and calls SDL_main on its SDL thread.
//          This file owns the product's Android policy: allowed orientations,
//          windowed fullscreen, where content lives, and the argv it composes
//          for LauncherMain. The engine modules it loads are packaged beside it.
//
//=============================================================================//

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_root.h"
#include "tier0/platform.h"
#include "tier0/threadtools.h"

DLL_IMPORT int LauncherMain( int argc, char **argv ); // liblauncher.so

namespace
{

// The directory holding the packaged native libraries, found from this
// library's own mapping (the APK extracts them, see AndroidManifest.xml).
bool GetNativeLibraryDir( char *out, size_t size )
{
	Dl_info info = {};
	if ( !dladdr( reinterpret_cast<void *>( &GetNativeLibraryDir ), &info ) || !info.dli_fname )
		return false;
	snprintf( out, size, "%s", info.dli_fname );
	char *slash = strrchr( out, '/' );
	if ( !slash )
		return false;
	*slash = '\0';
	return true;
}

} // namespace

extern "C" int SDL_main( int, char ** )
{
	// SDL_main runs on SDL's thread, not the Java thread that loaded tier0.
	DeclareCurrentThreadIsMainThread();

	// Rotation: landscape either way up, as the user's rotation lock allows
	// (user direction, 2026-10-06). SDL requests USER_LANDSCAPE from it.
	SDL_SetHint( SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight" );
	// Fingers reach the game only as touch events: the client's touch controls
	// (game/client/touch.cpp) and VGUI's finger handling. SDL's synthesized
	// mouse would otherwise turn every look drag into a left click (+attack).
	SDL_SetHint( SDL_HINT_TOUCH_MOUSE_EVENTS, "0" );

	static char libraryDir[PATH_MAX];
	if ( !GetNativeLibraryDir( libraryDir, sizeof( libraryDir ) ) )
	{
		SDL_Log( "Source: cannot locate the native library directory" );
		return 1;
	}
	// Content (game directories, commandline.txt) lives in the app's external
	// files directory: adb-pushable and needing no storage permission.
	const char *contentDir = SDL_GetAndroidExternalStoragePath();
	const char *internalDir = SDL_GetAndroidInternalStoragePath();
	if ( !contentDir || !internalDir )
	{
		SDL_Log( "Source: app storage is unavailable: %s", SDL_GetError() );
		return 1;
	}
	static char dataDir[PATH_MAX];
	snprintf( dataDir, sizeof( dataDir ), "%s", internalDir );
	if ( char *slash = strrchr( dataDir, '/' ) )
		*slash = '\0'; // The app data root; fonts are read from its files/.

	// The filesystem and module loader read these (public/filesystem_init.cpp,
	// tier1/interface.cpp, launcher/launcher.cpp).
	setenv( "APP_LIB_PATH", libraryDir, 1 );
	setenv( "VALVE_GAME_PATH", contentDir, 1 );
	setenv( "APP_DATA_PATH", dataDir, 1 );
	// Game directories (-game) are relative to the content root, as hl2.sh
	// starts the desktop client in its install directory.
	if ( chdir( contentDir ) != 0 )
	{
		SDL_Log( "Source: cannot enter the content directory %s", contentDir );
		return 1;
	}
	// The app's own UI art ships in the APK's assets.
	approot::InstallTouchIcons( "", ANDROID_DEFAULT_GAME );

	static char program[PATH_MAX];
	snprintf( program, sizeof( program ), "%s/hl2_linux", libraryDir );
	static char *argv[approot::kMaxArgs];
	int argc = 0;
	argv[argc++] = program;
	argv[argc++] = const_cast<char *>( "-game" );
	argv[argc++] = const_cast<char *>( ANDROID_DEFAULT_GAME );
	argv[argc++] = const_cast<char *>( "-renderer" );
	argv[argc++] = const_cast<char *>( "native-vulkan" );
	argv[argc++] = const_cast<char *>( "-fullscreen" );
	// The system owns the surface size: rotation and foldable display swaps
	// resize the back buffer (engine/sys_getmodes.cpp).
	argv[argc++] = const_cast<char *>( "+mat_windowed_fullscreen" );
	argv[argc++] = const_cast<char *>( "1" );
	argv[argc++] = const_cast<char *>( "-nosteam" );
	argv[argc++] = const_cast<char *>( "-insecure" );
	argv[argc++] = const_cast<char *>( "-nouserclip" );
	// The material system renders on the main thread here, as before native
	// Vulkan supported the queued mode: its render thread is unmeasured on the
	// Android profiles (surface recreation, backgrounding, thermal budget).
	// commandline.txt may still ask for +mat_queue_mode 2.
	argv[argc++] = const_cast<char *>( "+mat_queue_mode" );
	argv[argc++] = const_cast<char *>( "0" );

	static char argumentsPath[PATH_MAX];
	static char argumentsStorage[4096];
	snprintf( argumentsPath, sizeof( argumentsPath ), "%s/commandline.txt", contentDir );
	argc = approot::AppendArgumentsFile(
	    argumentsPath, argv, argc, argumentsStorage, sizeof( argumentsStorage ) );
	argv[argc] = NULL;

	SDL_Log( "Source: libraries=%s content=%s data=%s", libraryDir, contentDir, dataDir );
	return LauncherMain( argc, argv );
}
