//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The Apple app entry, shared by macOS, iOS and tvOS. SDL3 owns main() here
// (SDL_main.h): it starts AppKit or UIKit and then calls this main() on the
// app's main thread. Every first-party module is linked into the app
// (static_composition.cpp); nothing is loaded at run time.
//
// The game content (platform/, <game>/, hl2/ and an optional
// commandline.txt) lives outside the bundle, in the directory the platform
// half names (apple_app.h): Documents on iOS, Library/Caches on tvOS and
// Application Support on macOS.
//
//=============================================================================//

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_root.h"
#include "apple_app.h"
#include "apple_thermal.h"
#include "static_composition.h"
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

// The default game's gameinfo.txt stands for the copied content. tvOS may
// purge Library/Caches while the app is not running, so this runs at every
// start.
bool HaveGameContent( const char *contentDir )
{
	char gameInfo[PATH_MAX];
	snprintf( gameInfo, sizeof( gameInfo ), "%s/%s/gameinfo.txt", contentDir, APPLE_DEFAULT_GAME );
	return access( gameInfo, R_OK ) == 0;
}

} // namespace

int main( int, char ** )
{
	DeclareCurrentThreadIsMainThread();

	appleapp::ConfigureSdl();
	// The device's thermal state in the log, on the frame stream's clock.
	AppleThermal_StartLogging();
	// Line-buffered, so the engine's console output reaches the device console
	// (xcrun devicectl ... --console) or the terminal as it happens rather than
	// in 4 KB blocks.
	setvbuf( stdout, NULL, _IOLBF, 0 );

	const char *bundle = SDL_GetBasePath();
	const char *content = appleapp::ContentDirectory();
	if ( !bundle || !content )
	{
		SDL_Log( "Source: app storage is unavailable: %s", SDL_GetError() );
		return 1;
	}
	static char bundleDir[PATH_MAX], contentDir[PATH_MAX];
	CopyDirectory( bundleDir, sizeof( bundleDir ), bundle );
	CopyDirectory( contentDir, sizeof( contentDir ), content );

	// The filesystem and launcher read these on the app-container platforms
	// (public/filesystem_init.cpp, launcher/launcher.cpp,
	// vgui2/vgui_surfacelib/linuxfont.cpp).
	setenv( "APP_LIB_PATH", bundleDir, 1 );
	setenv( "VALVE_GAME_PATH", contentDir, 1 );
	setenv( "APP_DATA_PATH", contentDir, 1 );
	// Game directories (-game) are relative to the content root.
	if ( chdir( contentDir ) != 0 || !HaveGameContent( contentDir ) )
	{
		SDL_Log( "Source: no %s/gameinfo.txt under %s", APPLE_DEFAULT_GAME, contentDir );
		appleapp::ReportMissingContent( contentDir );
		return 1;
	}
	appleapp::PrepareContent( bundleDir, contentDir );

	if ( !StaticComposition_BindGame() )
	{
		SDL_Log( "Source: failed to bind the linked game modules" );
		return 1;
	}
	if ( !appleapp::BindPlatformServices( contentDir ) )
	{
		SDL_Log( "Source: failed to bind the platform services" );
		return 1;
	}

	static char program[PATH_MAX];
	snprintf( program, sizeof( program ), "%s/hl2_launcher", bundleDir );
	static char *argv[approot::kMaxArgs];
	const int maxArgs = approot::kMaxArgs;
	static const char *const kLaunchArgs[] = { "-game", APPLE_DEFAULT_GAME, "-renderer",
	    "native-vulkan", "-nosteam", "-insecure", "-nouserclip",
	    // Box3D (RFC 0004) is the Apple products' physics provider (user
	    // decision, 2026-09-26), as in the desktop kiln profiles; its parallel step
	    // runs on the engine compute pool. IVP stays linked for comparison runs.
	    "-physics", "vphysics_box3d",
	    // The material system renders on the main thread until its render
	    // thread is measured on the Apple profiles; commandline.txt may ask
	    // for +mat_queue_mode 2.
	    "+mat_queue_mode", "0" };
	int argc = 0;
	argv[argc++] = program;
	for ( const char *arg : kLaunchArgs )
		argc = approot::AppendArgument( argv, argc, maxArgs, arg );
	argc = appleapp::AppendLaunchArguments( argv, argc, maxArgs, contentDir );

	static char argumentsPath[PATH_MAX];
	static char argumentsStorage[4096];
	snprintf( argumentsPath, sizeof( argumentsPath ), "%s/commandline.txt", contentDir );
	argc = approot::AppendArgumentsFile(
	    argumentsPath, argv, argc, argumentsStorage, sizeof( argumentsStorage ) );
	argc = appleapp::AppendDefaultArguments( argv, argc, maxArgs );
	argv[argc] = NULL;

	SDL_Log( "Source: bundle=%s content=%s", bundleDir, contentDir );
	// SDL keeps a UIKit app running after main returns; the game has quit, so
	// the process ends with its status, as the Android activity finishes.
	exit( LauncherMain( argc, argv ) );
}
