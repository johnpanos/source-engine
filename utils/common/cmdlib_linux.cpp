//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Linux host-tool output, logging, and cleanup for cmdlib.
//
//=============================================================================//

#include "cmdlib.h"
#include "utllinkedlist.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <vector>

extern FileHandle_t g_pLogFile;
extern CUtlLinkedList<CleanupFn, unsigned short> g_CleanupFunctions;
extern CUtlLinkedList<SpewHookFn, unsigned short> g_ExtraSpewHooks;

bool g_bSuppressPrintfOutput = false;

namespace
{
std::mutex g_OutputMutex;
std::atomic<bool> g_CleanupStarted{ false };
} // namespace

void CmdLib_FPrintf( FileHandle_t file, const char *format, ... )
{
	if ( !g_pFileSystem || !file )
		return;

	va_list args;
	va_start( args, format );
	va_list countArgs;
	va_copy( countArgs, args );
	const int size = std::vsnprintf( nullptr, 0, format, countArgs );
	va_end( countArgs );
	if ( size >= 0 )
	{
		std::vector<char> buffer( static_cast<size_t>( size ) + 1 );
		std::vsnprintf( buffer.data(), buffer.size(), format, args );
		g_pFileSystem->Write( buffer.data(), size, file );
	}
	va_end( args );
}

char *CmdLib_FGets( char *out, int outSize, FileHandle_t file )
{
	if ( !out || outSize < 1 || !g_pFileSystem || !file )
		return nullptr;

	int count = 0;
	while ( count < outSize - 1 )
	{
		char character;
		if ( g_pFileSystem->Read( &character, 1, file ) != 1 )
			break;
		out[count++] = character;
		if ( character == '\n' )
			break;
	}
	out[count] = '\0';
	return count ? out : nullptr;
}

SpewRetval_t CmdLib_SpewOutputFunc( SpewType_t type, const char *message )
{
	std::vector<SpewHookFn> hooks;
	{
		std::lock_guard<std::mutex> lock( g_OutputMutex );
		if ( !g_bSuppressPrintfOutput || type == SPEW_ERROR )
		{
			std::fputs( message, stdout );
			if ( type == SPEW_ERROR )
				std::fputc( '\n', stdout );
		}
		if ( g_pLogFile && g_pFileSystem )
		{
			CmdLib_FPrintf( g_pLogFile, "%s", message );
			g_pFileSystem->Flush( g_pLogFile );
		}
		FOR_EACH_LL( g_ExtraSpewHooks, index )
		hooks.push_back( g_ExtraSpewHooks[index] );
	}
	for ( SpewHookFn hook : hooks )
		hook( message );

	if ( type == SPEW_ERROR )
		CmdLib_Exit( 1 );
	return type == SPEW_ASSERT ? SPEW_DEBUGGER : SPEW_CONTINUE;
}

void InstallSpewFunction()
{
	setvbuf( stdout, nullptr, _IONBF, 0 );
	setvbuf( stderr, nullptr, _IONBF, 0 );
	SpewOutputFunc( CmdLib_SpewOutputFunc );
}

void InstallExtraSpewHook( SpewHookFn hook )
{
	std::lock_guard<std::mutex> lock( g_OutputMutex );
	g_ExtraSpewHooks.AddToTail( hook );
}

void InstallAllocationFunctions()
{
}

void SetSpewFunctionLogFile( const char *filename )
{
	if ( !g_pFileSystem )
		Error( "Cannot open compile-tool log without a filesystem\n" );
	{
		std::lock_guard<std::mutex> lock( g_OutputMutex );
		if ( g_pLogFile )
			g_pFileSystem->Close( g_pLogFile );
		g_pLogFile = g_pFileSystem->Open( filename, "a" );
		if ( g_pLogFile )
			CmdLib_FPrintf( g_pLogFile, "\n\n\n" );
	}
	if ( !g_pLogFile )
	{
		std::fprintf( stderr, "Cannot open compile-tool log: %s\n", filename );
		CmdLib_Exit( 1 );
	}
}

void CloseSpewFunctionLogFile()
{
	std::lock_guard<std::mutex> lock( g_OutputMutex );
	if ( g_pFileSystem && g_pLogFile )
		g_pFileSystem->Close( g_pLogFile );
	g_pLogFile = FILESYSTEM_INVALID_HANDLE;
}

void CmdLib_AtCleanup( CleanupFn callback )
{
	g_CleanupFunctions.AddToTail( callback );
}

void CmdLib_Cleanup()
{
	if ( g_CleanupStarted.exchange( true ) )
		return;
	CloseSpewFunctionLogFile();
	CmdLib_TermFileSystem();
	FOR_EACH_LL( g_CleanupFunctions, index )
	g_CleanupFunctions[index]();
}

void CmdLib_Exit( int exitCode )
{
	CmdLib_Cleanup();
	std::exit( exitCode );
}
