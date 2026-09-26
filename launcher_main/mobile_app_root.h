//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Helpers shared by the mobile application roots (android_main.cpp,
// ios_main.cpp): the development arguments file and the app's own UI art.
//
//=============================================================================//

#ifndef MOBILE_APP_ROOT_H
#define MOBILE_APP_ROOT_H

#include <stddef.h>

namespace mobileapp
{

const int kMaxArgs = 128;

// Appends the whitespace-separated arguments in path (no quoting), e.g.
// "-dev 1 +map testchmb_a_00", keeping argc below kMaxArgs. The tokens point
// into storage. Returns the new argc; a missing file adds nothing.
int AppendArgumentsFile( const char *path, char **argv, int argc, char *storage, size_t size );

// Installs the touch-control icons (tools/android/touch_icons.py) packaged at
// <assetRoot>touch/ into <gameDir>/custom/android_touch/, which the game's
// gameinfo mounts at boot. assetRoot is "" for the Android APK's assets and
// the bundle directory with a trailing '/' on iOS. Files are rewritten only
// when the packaged copies differ.
void InstallTouchIcons( const char *assetRoot, const char *gameDir );

} // namespace mobileapp

#endif // MOBILE_APP_ROOT_H
