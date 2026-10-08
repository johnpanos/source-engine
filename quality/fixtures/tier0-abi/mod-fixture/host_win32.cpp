// R103 mod-fixture host for Windows PE: host.cpp's steps. It is linked against
// tier0.dll and the kept mod_fixture.dll (MinGW links the DLLs directly), so
// the Windows loader loads both with the process and no explicit loader call
// is made. It re-executes itself with the command line "modhost -modfixture
// 42" (Windows Tier 0 reads the process's own line; there is no
// Plat_SetCommandLine), gives CommandLine_Tier0 that line as the launcher does,
// runs the mod and reports its result as checks-v1. It uses no Tier 0 headers.
//
//   host_win32.exe      (beside tier0.dll and mod_fixture.dll)
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

extern "C" __declspec( dllimport ) void *CommandLine_Tier0();
extern "C" __declspec( dllimport ) int ModFixture_Run( char *report, int size );

namespace
{
template <typename T> T Find( HMODULE module, const char *name )
{
	return reinterpret_cast<T>( reinterpret_cast<void *>( GetProcAddress( module, name ) ) );
}
} // namespace

int main( int argc, char **argv )
{
	(void)argc;
	(void)argv;
	char child[8] = {};
	if ( GetEnvironmentVariableA( "R103_HOST_CHILD", child, sizeof( child ) ) == 0 )
	{
		SetEnvironmentVariableA( "R103_HOST_CHILD", "1" );
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
	void *( *get )() = CommandLine_Tier0;
	const char *line = GetCommandLineA();
	// CreateCmdLine( const char * ) is declared first, but MSVC lays out
	// overloaded virtuals in reverse, so it is the second ICommandLine slot. On
	// x64 the this pointer is the first argument; a plain call matches.
	void **object = static_cast<void **>( get() );
	typedef void ( *CreateCmdLine )( void *, const char * );
	reinterpret_cast<CreateCmdLine>( ( *reinterpret_cast<void ***>( object ) )[1] )( object, line );

	int ( *run )( char *, int ) = ModFixture_Run;
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
