//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The single owner of the debug API command-line policy, shared by the
// launcher and dedicated composition roots:
//
//   -debugapi unix:/abs/path.sock      serve on that Unix socket
//   -debugapi unix                     $XDG_RUNTIME_DIR/source-debugapi-<pid>.sock
//   -debugapi-framing newline|content-length   (default newline)
//
// Without -debugapi nothing is bound. Requesting it from a product built
// without --debug-api=enabled, or naming an unknown transport or framing,
// fails composition instead of silently running without the API.
//
//=============================================================================//

#ifndef ENGINE_DEBUGAPI_ROOT_H
#define ENGINE_DEBUGAPI_ROOT_H

#include <cstdlib>

#include "engine/debugapi_binding.h"
#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "tier0/platform.h"
#include "tier1/strtools.h"
#ifdef SOURCE_DEBUGAPI
#include <unistd.h>

#include "debugapi/debugapi_framing.h"
#include "debugapi/debugapi_transport.h"
#endif

inline bool DebugApi_BindFromCommandLine( const DebugApiComposedProvider *providers, size_t count )
{
	if ( !CommandLine()->FindParm( "-debugapi" ) )
		return true;
#ifndef SOURCE_DEBUGAPI
	(void)providers;
	(void)count;
	Warning( "-debugapi requires a product built with --debug-api=enabled.\n" );
	return false;
#else
	const char *uri = CommandLine()->ParmValue( "-debugapi", "" );
	const char *colon = Q_strstr( uri, ":" );
	const int schemeLength = colon ? int( colon - uri ) : Q_strlen( uri );
	char scheme[16] = {};
	if ( schemeLength < (int)sizeof( scheme ) )
		Q_strncpy( scheme, uri, schemeLength + 1 );
	const char *address = colon ? colon + 1 : "";

	const debugapi::TransportProvider *transports[] = { DebugApi_UnixSocketTransport() };
	const debugapi::TransportProvider *transport = NULL;
	for ( unsigned int i = 0; i < ARRAYSIZE( transports ); ++i )
	{
		if ( transports[i] && !Q_stricmp( scheme, transports[i]->scheme ) )
			transport = transports[i];
	}
	if ( !transport )
	{
		Warning( "Debug API transport '%s' is not available (use -debugapi unix:/path.sock).\n",
		    uri );
		return false;
	}

	char defaultAddress[256];
	if ( !address[0] && !Q_stricmp( transport->scheme, "unix" ) )
	{
		const char *runtimeDir = getenv( "XDG_RUNTIME_DIR" );
		if ( !runtimeDir || !runtimeDir[0] )
		{
			Warning( "-debugapi unix needs XDG_RUNTIME_DIR or an explicit socket path.\n" );
			return false;
		}
		Q_snprintf( defaultAddress, sizeof( defaultAddress ), "%s/source-debugapi-%d.sock",
		    runtimeDir, (int)getpid() );
		address = defaultAddress;
	}

	const debugapi::FramingProvider *framings[] = {
	    DebugApi_NewlineFraming(), DebugApi_ContentLengthFraming() };
	const char *requestedFraming =
	    CommandLine()->ParmValue( "-debugapi-framing", framings[0]->name );
	const debugapi::FramingProvider *framing = NULL;
	for ( unsigned int i = 0; i < ARRAYSIZE( framings ); ++i )
	{
		if ( framings[i] && !Q_stricmp( requestedFraming, framings[i]->name ) )
			framing = framings[i];
	}
	if ( !framing )
	{
		Warning( "Debug API framing '%s' is not available (newline, content-length).\n",
		    requestedFraming );
		return false;
	}

	DebugApiSelection selection;
	selection.transport = transport;
	selection.address = address;
	selection.framing = framing;
	selection.providers = providers;
	selection.providerCount = count;
	if ( !Engine_BindDebugApi( &selection ) )
	{
		Warning( "The engine rejected the debug API selection '%s'.\n", uri );
		return false;
	}
	return true;
#endif
}

#endif // ENGINE_DEBUGAPI_ROOT_H
