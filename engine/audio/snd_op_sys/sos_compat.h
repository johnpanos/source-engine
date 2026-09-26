//============ Copyright (c) Valve Corporation, All rights reserved. ============
//
// Purpose: Adapts the Portal 2 / CS:GO-era sound operator system (SOS) and
//          mixer sources to this engine: it has one listener (no split screen)
//          and no tier0 logging channels.
//
//===============================================================================

#ifndef SOS_COMPAT_H
#define SOS_COMPAT_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/dbg.h"
#include "Color.h"
#include "SoundEmitterSystem/isoundemittersystementries.h"

// One local listener.
#ifndef MAX_SPLITSCREEN_CLIENTS
#define MAX_SPLITSCREEN_CLIENTS 1
#endif
#ifndef FOR_EACH_VALID_SPLITSCREEN_PLAYER
#define FOR_EACH_VALID_SPLITSCREEN_PLAYER( iteratorName )                                          \
	for ( int iteratorName = 0; iteratorName < MAX_SPLITSCREEN_CLIENTS; ++iteratorName )
#endif

// The CS:GO sources log through named channels, optionally with a color.
// Route both forms to the console.
#ifndef DECLARE_LOGGING_CHANNEL
#define DECLARE_LOGGING_CHANNEL( name )
#endif
#undef DEFINE_LOGGING_CHANNEL_NO_TAGS
#define DEFINE_LOGGING_CHANNEL_NO_TAGS( ... )

void SOS_LogMsg( const Color &color, PRINTF_FORMAT_STRING const char *pFormat, ... )
    FMTFUNCTION( 2, 3 );
void SOS_LogMsg( PRINTF_FORMAT_STRING const char *pFormat, ... ) FMTFUNCTION( 1, 2 );
void SOS_LogWarning( const Color &color, PRINTF_FORMAT_STRING const char *pFormat, ... )
    FMTFUNCTION( 2, 3 );
void SOS_LogWarning( PRINTF_FORMAT_STRING const char *pFormat, ... ) FMTFUNCTION( 1, 2 );

#undef Log_Msg
#undef Log_Warning
#define Log_Msg( channel, ... ) SOS_LogMsg( __VA_ARGS__ )
#define Log_Warning( channel, ... ) SOS_LogWarning( __VA_ARGS__ )

// The engine reaches the mod's sound emitter (loaded as an app system for the
// game DLLs) through the app system factory once it exists. NULL before then.
// The operators use only its version 2 entry methods (VSoundEmitterEntries001,
// which the sound emitter answers through QueryInterface). NULL when the sound
// emitter has none: the operator system then stays off and version 2 entries
// play their wave like version 1 entries.
class ISoundEmitterSystemBase;
ISoundEmitterSystemBase *S_GetSoundEmitterSystem();
ISoundEmitterSystemEntries *S_GetSoundEmitterEntries();
#define g_pSoundEmitterSystem S_GetSoundEmitterEntries()

// Console platform branches of the operators never apply here.
#ifndef IsX360
#define IsX360() false
#endif
#ifndef IsPS3
#define IsPS3() false
#endif

#endif // SOS_COMPAT_H
