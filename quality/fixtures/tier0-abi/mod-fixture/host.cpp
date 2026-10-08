// R103 mod-fixture host: loads a Tier 0 build, sets its command line, loads the
// kept mod binary and reports its result as checks-v1. It uses no Tier 0
// headers, and loads through the platform's instrumented POSIX loader.
#include "platform/contracts/dynamic_library.h"
#include "platform/posix/dynamic_library_provider.h"

#include <stdio.h>
#include <stdlib.h>

namespace
{

class CQuietObserver final : public platform::IDynamicLibraryObserver
{
public:
	void OnLoad( const char *, const char *, const platform::IDynamicLibrary *,
		const platform::DynamicLibraryError & ) override
	{
	}
	void OnFindSymbol( const platform::IDynamicLibrary *, const char *, const void *,
		const platform::DynamicLibraryError & ) override
	{
	}
	void OnUnload( const platform::IDynamicLibrary * ) override {}
};

} // namespace

int main( int argc, char **argv )
{
	if ( argc != 3 )
	{
		printf( "usage: host <libtier0.so> <libmod_fixture.so>\nCONFORMANCE 1 1\n" );
		return 1;
	}
	CQuietObserver observer;
	auto loader = platform::CreatePosixDynamicLibraryLoader( observer );
	platform::IDynamicLibrary *tier0 = loader->Load( argv[1], nullptr );
	if ( tier0 == nullptr )
	{
		printf( "FAIL load tier0\nCONFORMANCE 1 1\n" );
		return 1;
	}
	typedef void ( *SetCommandLine )( const char * );
	typedef void *( *GetCommandLine )();
	auto set = reinterpret_cast<SetCommandLine>( tier0->FindSymbol( "Plat_SetCommandLine", nullptr ) );
	auto get = reinterpret_cast<GetCommandLine>( tier0->FindSymbol( "CommandLine_Tier0", nullptr ) );
	if ( set == nullptr || get == nullptr )
	{
		printf( "FAIL missing command-line exports\nCONFORMANCE 1 1\n" );
		return 1;
	}
	const char *line = "modhost -modfixture 42";
	set( line );
	// CreateCmdLine( const char * ) is the first ICommandLine slot.
	void **object = static_cast<void **>( get() );
	typedef void ( *CreateCmdLine )( void *, const char * );
	reinterpret_cast<CreateCmdLine>( ( *reinterpret_cast<void ***>( object ) )[0] )( object, line );

	platform::IDynamicLibrary *mod = loader->Load( argv[2], nullptr );
	if ( mod == nullptr )
	{
		printf( "FAIL load mod\nCONFORMANCE 1 1\n" );
		return 1;
	}
	typedef int ( *Run )( char *, int );
	auto run = reinterpret_cast<Run>( mod->FindSymbol( "ModFixture_Run", nullptr ) );
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
	// Tier 0 and the mod stay loaded until exit, as in a game process.
	fflush( stdout );
	_Exit( failures ? 1 : 0 );
}
