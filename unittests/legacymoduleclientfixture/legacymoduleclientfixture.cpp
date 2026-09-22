// Client compiled against frozen pre-Phase-A loader declarations.

#include "legacy_loader_abi_2013.h"
#include "legacy_module_client_fixture.h"

#include <string.h>

#if defined( _WIN32 )
#define LEGACY_CLIENT_EXPORT extern "C" __declspec( dllexport )
#else
#define LEGACY_CLIENT_EXPORT \
	extern "C" __attribute__(( visibility( "default" ) ))
#endif

class CPhaseALegacyLoaderClient : public IPhaseALegacyLoaderClient
{
public:
	virtual bool RunLegacyLoaderAbiProbe()
	{
		CSysModule *pModule = Sys_LoadModule( "moduleloadfixture" );
		if ( !pModule )
			return false;

		CreateInterfaceFn pFactory = Sys_GetFactory( pModule );
		void *pInterface = pFactory
			? pFactory( "PhaseALoaderFixture001", 0 )
			: 0;
		Sys_UnloadModule( pModule );
		return pInterface != 0;
	}
};

static CPhaseALegacyLoaderClient g_LegacyLoaderClient;

LEGACY_CLIENT_EXPORT void *CreateInterface(
	const char *pName,
	int *pReturnCode )
{
	if ( pName &&
		strcmp( pName, PHASE_A_LEGACY_LOADER_CLIENT_INTERFACE ) == 0 )
	{
		if ( pReturnCode )
			*pReturnCode = 0;
		return &g_LegacyLoaderClient;
	}
	if ( pReturnCode )
		*pReturnCode = 1;
	return 0;
}
