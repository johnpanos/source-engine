//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Helpers shared by the app-container roots (android_main.cpp,
// apple_main.cpp): the launch arguments, the development arguments file and
// the app's own UI art.
//
//=============================================================================//

#ifndef APP_ROOT_H
#define APP_ROOT_H

#include <stddef.h>

namespace approot
{

const int kMaxArgs = 128;

// Appends argument when argc stays below maxArgs (the room for the NULL
// terminator); returns the new argc. The pointer is stored, not copied.
int AppendArgument( char **argv, int argc, int maxArgs, const char *argument );

// Whether argv[1..argc) holds argument, compared without case.
bool HasArgument( char *const *argv, int argc, const char *argument );

// Appends the whitespace-separated arguments in path (no quoting), e.g.
// "-dev 1 +map testchmb_a_00", keeping argc below kMaxArgs. The tokens point
// into storage. Returns the new argc; a missing file adds nothing.
int AppendArgumentsFile( const char *path, char **argv, int argc, char *storage, size_t size );

// Installs the touch-control icons (tools/android/touch_icons.py) packaged at
// <assetRoot>touch/ into <gameDir>/custom/android_touch/, which the game's
// gameinfo mounts at boot. assetRoot is "" for the Android APK's assets and
// the bundle directory with a trailing '/' on iOS and tvOS. Files are
// rewritten only when the packaged copies differ.
void InstallTouchIcons( const char *assetRoot, const char *gameDir );

} // namespace approot

#endif // APP_ROOT_H
