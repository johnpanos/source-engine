// Client compiled against frozen pre-Phase-A loader declarations.

#include "legacy_loader_abi_2013.h"
#include "legacy_module_client_fixture.h"

#if defined( _WIN32 )
#define LEGACY_CLIENT_EXPORT extern "C" __declspec( dllexport )
#else
#define LEGACY_CLIENT_EXPORT \
	extern "C" __attribute__(( visibility( "default" ) ))
#endif

LEGACY_CLIENT_EXPORT int PhaseALegacyLoaderClientProbe()
{
	CSysModule *pModule = Sys_LoadModule( "moduleloadfixture" );
	if ( !pModule )
		return 0;

	CreateInterfaceFn pFactory = Sys_GetFactory( pModule );
	void *pInterface = pFactory
		? pFactory( "PhaseALoaderFixture001", 0 )
		: 0;
	Sys_UnloadModule( pModule );
	return pInterface != 0;
}
