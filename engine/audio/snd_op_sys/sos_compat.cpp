//============ Copyright (c) Valve Corporation, All rights reserved. ============
//
// Purpose: Console output for the sound operator system (see sos_compat.h).
//
//===============================================================================

#include "tier0/platform.h"
#include "tier1/strtools.h"
#include "sos_compat.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SOS_FORMAT_MESSAGE( buffer, format )                                                       \
	char buffer[2048];                                                                             \
	va_list args;                                                                                  \
	va_start( args, format );                                                                      \
	V_vsnprintf( buffer, sizeof( buffer ), format, args );                                         \
	va_end( args );

void SOS_LogMsg( const Color &color, const char *pFormat, ... )
{
	SOS_FORMAT_MESSAGE( message, pFormat );
	ConColorMsg( color, "%s", message );
}

void SOS_LogMsg( const char *pFormat, ... )
{
	SOS_FORMAT_MESSAGE( message, pFormat );
	Msg( "%s", message );
}

void SOS_LogWarning( const Color &color, const char *pFormat, ... )
{
	SOS_FORMAT_MESSAGE( message, pFormat );
	Warning( "%s", message );
}

void SOS_LogWarning( const char *pFormat, ... )
{
	SOS_FORMAT_MESSAGE( message, pFormat );
	Warning( "%s", message );
}
