//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The iOS app entry. SDL3 owns main() here (SDL_main.h): it starts UIKit and
// then calls this main() on the app's main thread. Every first-party module is
// linked into the app (static_composition.cpp); nothing is loaded at run time.
//
// Content (platform/, portal/, hl2/ and an optional commandline.txt) lives in
// the app's Documents container, which the Mac copies it into
// (xcrun devicectl ... --domain-type appDataContainer) and which the Files app
// can reach (UIFileSharingEnabled). tvOS gives apps no persistent storage
// outside the bundle, so there the content lives in Library/Caches, which the
// system may purge when storage runs low (the tvOS profile's content policy).
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

// The default game's gameinfo.txt stands for the copied content. On tvOS the
// system may purge Library/Caches while the app is not running, so this runs
// at every start.
bool HaveGameContent( const char *contentDir )
{
	char gameInfo[PATH_MAX];
	snprintf( gameInfo, sizeof( gameInfo ), "%s/%s/gameinfo.txt", contentDir, IOS_DEFAULT_GAME );
	return access( gameInfo, R_OK ) == 0;
}

// Says what is missing and how to restore it, instead of failing later inside
// the engine's filesystem setup.
void ReportMissingContent( const char *contentDir )
{
#if defined( PLATFORM_TVOS )
	const char *reason =
	    "The game content in this app's cache is missing. tvOS may have removed it to free "
	    "storage. Copy it again from the Mac:\n\n"
	    "./ios-deploy.sh build-tvos/Portal.app --device tv --with-content";
#else
	const char *reason =
	    "The game content is missing from this app's Documents folder. Copy it with the Files "
	    "app, or from the Mac:\n\n"
	    "./ios-deploy.sh --with-content";
#endif
	SDL_Log( "Source: no %s/gameinfo.txt under %s", IOS_DEFAULT_GAME, contentDir );
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Game content missing", reason, NULL );
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
#if defined( PLATFORM_TVOS )
	// The Siri Remote is not a gamepad here, so it never takes the engine's one
	// active gamepad slot from a real controller (it sends keys instead).
	SDL_SetHint( SDL_HINT_TV_REMOTE_AS_JOYSTICK, "0" );
#endif
	// Line-buffered, so the engine's console output reaches the device console
	// (xcrun devicectl ... --console) as it happens rather than in 4 KB blocks.
	setvbuf( stdout, NULL, _IOLBF, 0 );

	const char *bundle = SDL_GetBasePath();
#if defined( PLATFORM_TVOS )
	// HOME is the app's data container.
	static char caches[PATH_MAX];
	const char *home = getenv( "HOME" );
	if ( home )
		snprintf( caches, sizeof( caches ), "%s/Library/Caches", home );
	const char *content = home ? caches : NULL;
#else
	const char *content = SDL_GetUserFolder( SDL_FOLDER_DOCUMENTS );
#endif
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
		ReportMissingContent( contentDir );
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

#if defined( PLATFORM_TVOS )
	// Apple TV render defaults (user direction, 2026-09-26): 1080p scaled up to
	// a 4K display by the system, no MSAA, and the Video Advanced dialog's Low
	// values (gameui/OptionsSubVideo.cpp); HDR is left as configured. The '+'
	// settings run after config.cfg, so a desktop config copied with the
	// content cannot raise them; commandline.txt comes later and can.
	static const char *const kTvLaunchArgs[] = { "-nohighdpi", "-mat_antialias", "1" };
	static const char *const kTvSettings[][2] = {
	    { "+mat_antialias", "1" },
	    { "+mat_aaquality", "0" },
	    { "+r_rootlod", "2" },
	    { "+mat_picmip", "2" },
	    { "+mat_trilinear", "0" },
	    { "+mat_forceaniso", "1" },
	    { "+r_shadowrendertotexture", "0" },
	    { "+r_flashlightdepthtexture", "0" },
	    { "+mat_reducefillrate", "1" },
	    { "+r_waterforceexpensive", "0" },
	    { "+r_waterforcereflectentities", "0" },
	    { "+mat_colorcorrection", "0" },
	    { "+mat_motion_blur_enabled", "0" },
	};
	for ( const char *arg : kTvLaunchArgs )
		argv[argc++] = const_cast<char *>( arg );
	for ( const auto &setting : kTvSettings )
	{
		argv[argc++] = const_cast<char *>( setting[0] );
		argv[argc++] = const_cast<char *>( setting[1] );
	}
#endif

	static char argumentsPath[PATH_MAX];
	static char argumentsStorage[4096];
	snprintf( argumentsPath, sizeof( argumentsPath ), "%s/commandline.txt", contentDir );
	argc = mobileapp::AppendArgumentsFile(
	    argumentsPath, argv, argc, argumentsStorage, sizeof( argumentsStorage ) );
	argv[argc] = NULL;

	SDL_Log( "Source: bundle=%s content=%s", bundleDir, contentDir );
	// SDL keeps a UIKit app running after main returns; the game has quit, so
	// the process ends with its status, as the Android activity finishes.
	exit( LauncherMain( argc, argv ) );
}
