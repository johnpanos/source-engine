// R103 mod-fixture host: loads a Tier 0 build, sets its command line, loads the
// kept mod binary and reports its result as checks-v1. No Tier 0 headers.
#include <dlfcn.h>
#include <stdio.h>
#include <string>

int main( int argc, char **argv )
{
	if ( argc != 3 )
	{
		printf( "usage: host <libtier0.so> <libmod_fixture.so>\nCONFORMANCE 1 1\n" );
		return 1;
	}
	void *tier0 = dlopen( argv[1], RTLD_NOW | RTLD_GLOBAL );
	if ( tier0 == nullptr )
	{
		printf( "FAIL dlopen tier0: %s\nCONFORMANCE 1 1\n", dlerror() );
		return 1;
	}
	typedef void ( *SetCommandLine )( const char * );
	auto set = reinterpret_cast<SetCommandLine>( dlsym( tier0, "Plat_SetCommandLine" ) );
	typedef void *( *GetCommandLine )();
	auto get = reinterpret_cast<GetCommandLine>( dlsym( tier0, "CommandLine_Tier0" ) );
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

	void *mod = dlopen( argv[2], RTLD_NOW );
	if ( mod == nullptr )
	{
		printf( "FAIL dlopen mod: %s\nCONFORMANCE 1 1\n", dlerror() );
		return 1;
	}
	typedef int ( *Run )( char *, int );
	auto run = reinterpret_cast<Run>( dlsym( mod, "ModFixture_Run" ) );
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
	return failures ? 1 : 0;
}
