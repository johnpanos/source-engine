//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Normal executable entry point for the directly linked VRAD tool.
//
//=============================================================================//

#include <stdio.h>
#include <string.h>

#if defined( _WIN32 )
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "tier0/icommandline.h"
#include "tier1/strtools.h"
#include "vrad_tool_entry.h"

namespace
{
enum
{
	VRAD_PROCESS_ROUTING_UNAVAILABLE = 4,
};

#if defined( _WIN32 )
const char VRAD_PATH_SEPARATOR = '\\';
#else
const char VRAD_PATH_SEPARATOR = '/';
#endif

bool HasArgument( int argc, char **argv, const char *pArgument )
{
	for ( int i = 1; i < argc; ++i )
	{
		if ( !Q_stricmp( argv[i], pArgument ) )
			return true;
	}

	return false;
}

bool MakeFullPath( const char *pInput, char *pOutput, int outputLength )
{
	if ( pInput[0] == '/' || pInput[0] == '\\' || pInput[1] == ':' )
	{
		Q_strncpy( pOutput, pInput, outputLength );
		return true;
	}

#if defined( _WIN32 )
	if ( !_getcwd( pOutput, outputLength ) )
#else
	if ( !getcwd( pOutput, outputLength ) )
#endif
	{
		pOutput[0] = '\0';
		return false;
	}

	char separator[2] = { VRAD_PATH_SEPARATOR, '\0' };
	Q_strncat( pOutput, separator, outputLength, COPY_ALL_CHARACTERS );
	Q_strncat( pOutput, pInput, outputLength, COPY_ALL_CHARACTERS );
	return true;
}

bool GetRedirectTarget( const char *pExecutable, char *pTarget, int targetLength )
{
	char fullPath[512];
	char redirectFilename[512];
	if ( !MakeFullPath( pExecutable, fullPath, sizeof( fullPath ) ) )
		return false;

	Q_StripFilename( fullPath );
	Q_snprintf( redirectFilename, sizeof( redirectFilename ), "%s%cvrad.redirect", fullPath,
	    VRAD_PATH_SEPARATOR );

	FILE *pFile = fopen( redirectFilename, "rt" );
	if ( !pFile )
		return false;

	pTarget[0] = '\0';
	const bool hasTarget = fgets( pTarget, targetLength, pFile ) != NULL;
	fclose( pFile );
	if ( !hasTarget )
		return false;

	char *pLineEnd = strpbrk( pTarget, "\r\n" );
	if ( pLineEnd )
		*pLineEnd = '\0';

	return pTarget[0] != '\0';
}
} // namespace

int main( int argc, char **argv )
{
	CommandLine()->CreateCmdLine( argc, argv );

	if ( HasArgument( argc, argv, "-both" ) )
	{
		fprintf( stderr,
		    "vrad: -both requires isolated -ldr and -hdr child processes. "
		    "Structured process routing is not installed for VRAD yet; no lighting was run.\n" );
		return VRAD_PROCESS_ROUTING_UNAVAILABLE;
	}

	char redirectTarget[512];
	if ( GetRedirectTarget( argv[0], redirectTarget, sizeof( redirectTarget ) ) )
	{
		fprintf( stderr,
		    "vrad: vrad.redirect requests alternate tool '%s'. Alternate-version execution "
		    "requires the structured child-process provider; no lighting was run.\n",
		    redirectTarget );
		return VRAD_PROCESS_ROUTING_UNAVAILABLE;
	}

	return VRad_RunCommandLine( argc, argv );
}
