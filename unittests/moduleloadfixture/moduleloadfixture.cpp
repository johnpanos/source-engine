// Small shared library used by the RFC 0001 loader telemetry tests.

#include <string.h>

#if defined( _WIN32 )
#define MODULE_LOAD_FIXTURE_EXPORT extern "C" __declspec( dllexport )
#else
#define MODULE_LOAD_FIXTURE_EXPORT extern "C" __attribute__(( visibility( "default" ) ))
#endif

static int g_ModuleLoadFixtureValue = 0x1A2B3C4D;

MODULE_LOAD_FIXTURE_EXPORT void *CreateInterface(
	const char *pName,
	int *pReturnCode )
{
	if ( pName && strcmp( pName, "PhaseALoaderFixture001" ) == 0 )
	{
		if ( pReturnCode )
			*pReturnCode = 0;
		return &g_ModuleLoadFixtureValue;
	}

	if ( pReturnCode )
		*pReturnCode = 1;
	return 0;
}

MODULE_LOAD_FIXTURE_EXPORT int PhaseALoaderFixtureSymbol()
{
	return g_ModuleLoadFixtureValue;
}
