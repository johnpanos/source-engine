//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The iOS and tvOS half of the Apple app root (apple_app.h).
//
// Content lives in the app's Documents container, which the Mac copies it
// into (xcrun devicectl ... --domain-type appDataContainer) and which the
// Files app can reach (UIFileSharingEnabled). tvOS gives apps no persistent
// storage outside the bundle, so there the content lives in Library/Caches,
// which the system may purge when storage runs low (the tvOS profile's
// content policy).
//
//=============================================================================//

#include <SDL3/SDL.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../platform/achievements/queued_achievement_service.h"
#include "../platform/apple/game_center_achievements.h"
#include "../platform/apple/user_defaults_record_store.h"
#include "../platform/records/file_record_store.h"
#include "app_root.h"
#include "apple_app.h"
#include "game/game_platform_services.h"
#include "static_composition.h"

namespace
{

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
	snprintf( gameDir, sizeof( gameDir ), "%s/%s", contentDir, APPLE_DEFAULT_GAME );
	platform::FileRecordStore cached( gameDir );
	auto old = cached.Load( kRecord );
	if ( !old )
		return;
	const bool imported = store.Commit( kRecord, old.Value() ).HasValue();
	SDL_Log(
	    "Source: %s %s from Library/Caches", imported ? "imported" : "failed to import", kRecord );
}

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
bool s_TvDefaultsWritten = false;

// Rewritten at every start into the game's cfg directory, so the settings
// above stay their only source.
bool WriteTvDefaultsConfig( const char *contentDir )
{
	char path[PATH_MAX];
	snprintf(
	    path, sizeof( path ), "%s/%s/cfg/%s", contentDir, APPLE_DEFAULT_GAME, kTvDefaultsConfig );
	FILE *file = fopen( path, "w" );
	if ( !file )
	{
		SDL_Log( "Source: cannot write %s; the Apple TV defaults are not applied", path );
		return false;
	}
	fputs( "// Written by the app at startup (launcher_main/apple_app_uikit.cpp).\n", file );
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

} // namespace

namespace appleapp
{

void ConfigureSdl()
{
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
}

const char *ContentDirectory()
{
#if defined( PLATFORM_TVOS )
	// HOME is the app's data container.
	static char caches[PATH_MAX];
	const char *home = getenv( "HOME" );
	if ( !home )
		return NULL;
	snprintf( caches, sizeof( caches ), "%s/Library/Caches", home );
	return caches;
#else
	return SDL_GetUserFolder( SDL_FOLDER_DOCUMENTS );
#endif
}

void ReportMissingContent( const char * )
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
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Game content missing", reason, NULL );
}

void PrepareContent( const char *bundleDir, const char *contentDir )
{
	// The app's own UI art ships in the bundle's touch/ directory.
	static char assetRoot[PATH_MAX];
	snprintf( assetRoot, sizeof( assetRoot ), "%s/", bundleDir );
	approot::InstallTouchIcons( assetRoot, APPLE_DEFAULT_GAME );
#if defined( PLATFORM_TVOS )
	s_TvDefaultsWritten = WriteTvDefaultsConfig( contentDir );
#else
	(void)contentDir;
#endif
}

bool BindPlatformServices( const char *contentDir )
{
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
#else
	(void)contentDir;
#endif
	if ( !StaticComposition_BindPlatformServices( services ) )
		return false;
	// tools/quality/game_center_e2e.py launches with this set to read what Game
	// Center holds for the app, without starting the engine.
	if ( getenv( "SOURCE_GAME_CENTER_AUDIT" ) )
	{
		platform::AuditGameCenter( 30.0 );
		exit( 0 );
	}
	return true;
}

int AppendLaunchArguments( char **argv, int argc, int maxArgs, const char * )
{
	static const char *const kArgs[] = {
	    "-fullscreen",
	    // The system owns the surface size (rotation, split view).
	    "+mat_windowed_fullscreen",
	    "1",
#if defined( PLATFORM_TVOS )
	    // Apple TV render defaults: 1080p scaled up to a 4K display by the
	    // system (-nohighdpi) and no MSAA at the first mode set, then the
	    // settings of WriteTvDefaultsConfig after config.cfg.
	    "-nohighdpi",
	    "-mat_antialias",
	    "1",
#endif
	};
	for ( const char *arg : kArgs )
		argc = approot::AppendArgument( argv, argc, maxArgs, arg );
#if defined( PLATFORM_TVOS )
	if ( s_TvDefaultsWritten )
	{
		argc = approot::AppendArgument( argv, argc, maxArgs, "+exec" );
		argc = approot::AppendArgument( argv, argc, maxArgs, kTvDefaultsConfig );
	}
#endif
	return argc;
}

int AppendDefaultArguments( char **argv, int argc, int maxArgs )
{
	// After commandline.txt, so a '+gameui_xbox 0' there keeps the touch UI
	// (the engine reads the first occurrence).
	if ( !approot::HasArgument( argv, argc, "+gameui_xbox" ) && argc + 2 < maxArgs &&
	     UseConsoleUI() )
	{
		argc = approot::AppendArgument( argv, argc, maxArgs, "+gameui_xbox" );
		argc = approot::AppendArgument( argv, argc, maxArgs, "1" );
		SDL_Log( "Source: console UI (gameui_xbox 1)" );
	}
#if !defined( PLATFORM_TVOS )
	// The iPhone presents in its extended (EDR) range through the core's
	// output (RFC 0016 "Output", user request 2026-09-29); commandline.txt may
	// ask for '+mat_hdr_output 0'.
	if ( !approot::HasArgument( argv, argc, "+mat_hdr_output" ) && argc + 2 < maxArgs )
	{
		argc = approot::AppendArgument( argv, argc, maxArgs, "+mat_hdr_output" );
		argc = approot::AppendArgument( argv, argc, maxArgs, "1" );
	}
#endif
	return argc;
}

} // namespace appleapp
