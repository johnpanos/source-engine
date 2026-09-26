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
#include <strings.h>
#include <unistd.h>

#include "../platform/achievements/queued_achievement_service.h"
#include "../platform/apple/game_center_achievements.h"
#include "../platform/apple/user_defaults_record_store.h"
#include "../platform/records/file_record_store.h"
#include "apple_thermal.h"
#include "game/game_platform_services.h"
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

#if defined( PLATFORM_TVOS )
// Builds before the user-defaults store kept the achievement state in the
// game directory under Library/Caches. Copies it into the store once, when
// the store has none, so an update keeps the player's achievements.
void ImportCachedPlayerRecords( platform::IRecordStore &store, const char *contentDir )
{
	const char *const kRecord = GAME_STATE_RECORD_KEY;
	auto current = store.Load( kRecord );
	if ( current || current.Error().code != platform::RecordStoreErrorCode::kNotFound )
		return;
	char gameDir[PATH_MAX];
	snprintf( gameDir, sizeof( gameDir ), "%s/%s", contentDir, IOS_DEFAULT_GAME );
	platform::FileRecordStore cached( gameDir );
	auto old = cached.Load( kRecord );
	if ( !old )
		return;
	const bool imported = store.Commit( kRecord, old.Value() ).HasValue();
	SDL_Log(
	    "Source: %s %s from Library/Caches", imported ? "imported" : "failed to import", kRecord );
}
#endif

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

#if defined( PLATFORM_TVOS )
// Apple TV defaults (user direction, 2026-09-26): no MSAA, the Video Advanced
// dialog's values (gameui/OptionsSubVideo.cpp) from Low up to what fits the
// 60 fps budget, no bloom and sound on; HDR is left as configured. Exec'd
// after config.cfg, so a desktop config copied with the content cannot
// override them; commandline.txt comes later and can. A cfg rather than '+'
// arguments keeps the engine's 512-character command line free for
// commandline.txt.
const char kTvDefaultsConfig[] = "tvos_defaults.cfg";
const char *const kTvDefaults[][2] = {
    { "mat_antialias", "1" },
    { "mat_aaquality", "0" },
    // Raised from the Low preset as far as the 60 fps budget allows (a greedy
    // sweep on the Apple TV 4K, 2026-09-26): high textures and models, 16x
    // anisotropic filtering, medium (render-to-texture) shadows. High shader
    // detail, color correction and MSAA missed the budget.
    { "r_rootlod", "0" },
    { "mat_picmip", "0" },
    { "mat_trilinear", "0" },
    { "mat_forceaniso", "16" },
    { "r_shadowrendertotexture", "1" },
    { "r_flashlightdepthtexture", "0" },
    { "mat_reducefillrate", "1" },
    { "r_waterforceexpensive", "0" },
    { "r_waterforcereflectentities", "0" },
    { "mat_colorcorrection", "0" },
    { "mat_motion_blur_enabled", "0" },
    // Bloom's downsample, blur and full-screen composite cost 1.35 ms of the
    // A15's GPU per frame at 1080p (frame_pacing_device.py, 2026-09-26).
    { "mat_disable_bloom", "1" },
    // Sound on at full game volume (user direction, 2026-09-26): the level is
    // archived in config.cfg, and a saved 0 (content copied from a desktop,
    // or a benchmark that muted it) must not silence the TV.
    { "volume", "1" },
};

// Rewritten at every start into the game's cfg directory, so the settings
// above stay their only source.
bool WriteTvDefaultsConfig( const char *contentDir )
{
	char path[PATH_MAX];
	snprintf(
	    path, sizeof( path ), "%s/%s/cfg/%s", contentDir, IOS_DEFAULT_GAME, kTvDefaultsConfig );
	FILE *file = fopen( path, "w" );
	if ( !file )
	{
		SDL_Log( "Source: cannot write %s; the Apple TV defaults are not applied", path );
		return false;
	}
	fputs( "// Written by the app at startup (launcher_main/ios_main.cpp).\n", file );
	for ( const auto &setting : kTvDefaults )
		fprintf( file, "%s %s\n", setting[0], setting[1] );
	return fclose( file ) == 0;
}
#endif

// The console GameUI (the Xbox 360 menus: gamepad focus, button hints, the
// controller options) instead of the desktop one, which needs a pointer: on
// the Apple TV always, and on iOS when a gamepad is connected at launch.
// GameUI chooses once, at startup, from gameui_xbox (the engine applies a
// '+gameui_xbox' argument before GameUI starts, host.cpp); a controller
// connected later takes effect at the next launch.
bool UseConsoleUI()
{
#if defined( PLATFORM_TVOS )
	return true;
#else
	if ( !SDL_InitSubSystem( SDL_INIT_GAMEPAD ) )
	{
		SDL_Log( "Source: no gamepad subsystem (%s); using the touch UI", SDL_GetError() );
		return false;
	}
	const bool bGamepad = SDL_HasGamepad();
	SDL_QuitSubSystem( SDL_INIT_GAMEPAD );
	return bGamepad;
#endif
}

bool HasArgument( char *const *argv, int argc, const char *argument )
{
	for ( int i = 1; i < argc; ++i )
	{
		if ( strcasecmp( argv[i], argument ) == 0 )
			return true;
	}
	return false;
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
	// The device's thermal state in the log, on the frame stream's clock.
	AppleThermal_StartLogging();
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

	// Platform services the game modules borrow for the life of the process.
	// Game Center reports achievements and shows its achievements screen on
	// both platforms. tvOS keeps player records in the user defaults, its only
	// persistent local storage; iOS keeps them in files in Documents.
	static platform::GameCenterAchievements s_GameCenter;
	static platform::QueuedAchievementService s_Achievements( s_GameCenter );
	s_GameCenter.Authenticate(
	    []( bool signedIn )
	    {
		    SDL_Log( "Source: Game Center %s", signedIn ? "signed in" : "signed out" );
		    s_Achievements.SetSignedIn( signedIn );
	    } );
	GamePlatformServices services;
	services.achievements = &s_Achievements;
#if defined( PLATFORM_TVOS )
	static platform::UserDefaultsRecordStore s_PlayerRecords;
	services.playerRecords = &s_PlayerRecords;
	ImportCachedPlayerRecords( s_PlayerRecords, contentDir );
#endif
	if ( !StaticComposition_BindPlatformServices( services ) )
	{
		SDL_Log( "Source: failed to bind the platform services" );
		return 1;
	}
	// tools/quality/game_center_e2e.py launches with this set to read what Game
	// Center holds for the app, without starting the engine.
	if ( getenv( "SOURCE_GAME_CENTER_AUDIT" ) )
	{
		platform::AuditGameCenter( 30.0 );
		exit( 0 );
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
	// Box3D (RFC 0004) is the Apple products' physics provider (user decision,
	// 2026-09-26), as in the desktop ./play; its parallel step runs on the
	// engine compute pool. IVP stays linked for comparison runs.
	argv[argc++] = const_cast<char *>( "-physics" );
	argv[argc++] = const_cast<char *>( "vphysics_box3d" );
	// The material system renders on the main thread, as on Android, until
	// its render thread is measured on the mobile profiles; commandline.txt
	// may ask for +mat_queue_mode 2.
	argv[argc++] = const_cast<char *>( "+mat_queue_mode" );
	argv[argc++] = const_cast<char *>( "0" );

#if defined( PLATFORM_TVOS )
	// Apple TV render defaults: 1080p scaled up to a 4K display by the system
	// (-nohighdpi) and no MSAA at the first mode set, then the settings of
	// WriteTvDefaultsConfig after config.cfg.
	static const char *const kTvLaunchArgs[] = { "-nohighdpi", "-mat_antialias", "1" };
	for ( const char *arg : kTvLaunchArgs )
		argv[argc++] = const_cast<char *>( arg );
	if ( WriteTvDefaultsConfig( contentDir ) )
	{
		argv[argc++] = const_cast<char *>( "+exec" );
		argv[argc++] = const_cast<char *>( kTvDefaultsConfig );
	}
#endif

	static char argumentsPath[PATH_MAX];
	static char argumentsStorage[4096];
	snprintf( argumentsPath, sizeof( argumentsPath ), "%s/commandline.txt", contentDir );
	argc = mobileapp::AppendArgumentsFile(
	    argumentsPath, argv, argc, argumentsStorage, sizeof( argumentsStorage ) );

	// After commandline.txt, so a '+gameui_xbox 0' there keeps the touch UI
	// (the engine reads the first occurrence).
	if ( !HasArgument( argv, argc, "+gameui_xbox" ) && UseConsoleUI() &&
	     argc + 2 < mobileapp::kMaxArgs )
	{
		argv[argc++] = const_cast<char *>( "+gameui_xbox" );
		argv[argc++] = const_cast<char *>( "1" );
		SDL_Log( "Source: console UI (gameui_xbox 1)" );
	}
	argv[argc] = NULL;

	SDL_Log( "Source: bundle=%s content=%s", bundleDir, contentDir );
	// SDL keeps a UIKit app running after main returns; the game has quit, so
	// the process ends with its status, as the Android activity finishes.
	exit( LauncherMain( argc, argv ) );
}
