// Process fixture for RFC 0001 Phase A retained-module shutdown telemetry.

#include "tier0/icommandline.h"
#include "tier0/native_module_load_telemetry.h"

int main( int argc, char **argv )
{
	CommandLine()->CreateCmdLine( argc, argv );

#if defined( POSIX )
	void *pModule = dlopen( "./libmoduleloadfixture.so", RTLD_NOW );
	if ( !pModule )
		return 1;
	if ( !dlsym( pModule, "PhaseALoaderFixtureSymbol" ) )
		return 2;
#elif defined( _WIN32 )
	HMODULE pModule = LoadLibraryA( "moduleloadfixture.dll" );
	if ( !pModule )
		return 1;
	if ( !GetProcAddress( pModule, "PhaseALoaderFixtureSymbol" ) )
		return 2;
#endif

	// Deliberately retain the handle. The process-shutdown reporter must emit
	// the failed unload result and measured lifetime for this record.
	return 0;
}
