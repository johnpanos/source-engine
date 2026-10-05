//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The platform half of the Apple app root (apple_main.cpp). One
// implementation is linked per platform: apple_app_uikit.cpp for iOS and
// tvOS, apple_app_macos.cpp for macOS. apple_main.cpp owns the launch
// sequence; these functions own what differs between the platforms.
//
//=============================================================================//

#ifndef APPLE_APP_H
#define APPLE_APP_H

namespace appleapp
{

// SDL hints that must be set before SDL starts a subsystem.
void ConfigureSdl();

// The directory holding the game content (platform/, <game>/, hl2/ and an
// optional commandline.txt), or NULL with the SDL error set.
const char *ContentDirectory();

// Tells the player the content is missing and how to restore it.
void ReportMissingContent( const char *contentDir );

// Writes what the platform keeps in the content directory before the
// engine starts (the app's own UI art, generated configs).
void PrepareContent( const char *bundleDir, const char *contentDir );

// Binds the platform services the game modules borrow for the life of the
// process (StaticComposition_BindPlatformServices).
bool BindPlatformServices( const char *contentDir );

// Appends the platform's launch arguments before commandline.txt, which can
// override them; returns the new argc.
int AppendLaunchArguments( char **argv, int argc, int maxArgs, const char *contentDir );

// Appends defaults that apply only when neither the launch arguments nor
// commandline.txt chose them; returns the new argc.
int AppendDefaultArguments( char **argv, int argc, int maxArgs );

} // namespace appleapp

#endif // APPLE_APP_H
