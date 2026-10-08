// R103 mod-fixture host for Windows PE: host.cpp's steps over LoadLibrary. It
// re-executes itself with the command line "modhost -modfixture 42" (Windows
// Tier 0 reads the process's own line; there is no Plat_SetCommandLine), loads
// a Tier 0 build, gives CommandLine_Tier0 that line as the launcher does, loads
// the kept mod DLL (which resolves tier0.dll from the host's search path) and
// reports its result as checks-v1. It uses no Tier 0 headers.
//
//   host_win32.exe <tier0.dll> <mod_fixture.dll>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

namespace
{
template <typename T> T Find( HMODULE module, const char *name )
{
	return reinterpret_cast<T>( reinterpret_cast<void *>( GetProcAddress( module, name ) ) );
}
} // namespace

int main( int argc, char **argv )
{
	char tier0Path[MAX_PATH] = {};
	char modPath[MAX_PATH] = {};
	if ( GetEnvironmentVariableA( "R103_HOST_TIER0", tier0Path, MAX_PATH ) == 0 )
	{
		if ( argc != 3 )
		{
			printf( "usage: host_win32 <tier0.dll> <mod_fixture.dll>\nCONFORMANCE 1 1\n" );
			return 1;
		}
		SetEnvironmentVariableA( "R103_HOST_TIER0", argv[1] );
		SetEnvironmentVariableA( "R103_HOST_MOD", argv[2] );
		char self[MAX_PATH] = {};
		GetModuleFileNameA( nullptr, self, MAX_PATH );
		char line[] = "modhost -modfixture 42";
		STARTUPINFOA startup = { sizeof( startup ) };
		PROCESS_INFORMATION process = {};
		if ( !CreateProcessA( self, line, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startup, &process ) )
		{
			printf( "FAIL re-exec (%lu)\nCONFORMANCE 1 1\n", GetLastError() );
			return 1;
		}
		WaitForSingleObject( process.hProcess, INFINITE );
		DWORD code = 1;
		GetExitCodeProcess( process.hProcess, &code );
		return static_cast<int>( code );
	}
	GetEnvironmentVariableA( "R103_HOST_MOD", modPath, MAX_PATH );
	HMODULE tier0 = LoadLibraryA( tier0Path );
	if ( tier0 == nullptr )
	{
		printf( "FAIL load tier0 (%lu)\nCONFORMANCE 1 1\n", GetLastError() );
		return 1;
	}
	auto get = Find<void *( * )()>( tier0, "CommandLine_Tier0" );
	if ( get == nullptr )
	{
		printf( "FAIL missing command-line exports\nCONFORMANCE 1 1\n" );
		return 1;
	}
	const char *line = GetCommandLineA();
	// CreateCmdLine( const char * ) is declared first, but MSVC lays out
	// overloaded virtuals in reverse, so it is the second ICommandLine slot. On
	// x64 the this pointer is the first argument; a plain call matches.
	void **object = static_cast<void **>( get() );
	typedef void ( *CreateCmdLine )( void *, const char * );
	reinterpret_cast<CreateCmdLine>( ( *reinterpret_cast<void ***>( object ) )[1] )( object, line );

	HMODULE mod = LoadLibraryA( modPath );
	if ( mod == nullptr )
	{
		printf( "FAIL load mod (%lu)\nCONFORMANCE 1 1\n", GetLastError() );
		return 1;
	}
	auto run = Find<int ( * )( char *, int )>( mod, "ModFixture_Run" );
	if ( run == nullptr )
	{
		printf( "FAIL ModFixture_Run missing\nCONFORMANCE 1 1\n" );
		return 1;
	}
	char report[4096];
	const int result = run( report, sizeof( report ) );
	const int checks = result / 1000;
	const int failures = result % 1000;
	fputs( report, stdout );
	printf( "%s mod fixture: %d checks, %d failures\nCONFORMANCE %d %d\n", failures ? "FAIL" : "ok", checks,
	        failures, checks, failures );
	fflush( stdout );
	_Exit( failures ? 1 : 0 );
}
