//========= Copyright Valve Corporation, All rights reserved. ============//
//
// RFC 0001 Phase A loader telemetry tests.
//
//=============================================================================//

#include "unitlib/unitlib.h"
#include "tier0/dbg.h"
#include "tier0/native_module_load_telemetry.h"
#include "tier0/threadtools.h"
#include "tier1/interface.h"
#include "tier1/module_load_telemetry.h"
#include "tier1/strtools.h"
#include "../legacymoduleclientfixture/legacy_module_client_fixture.h"
#include "tier1/dynamic_library_telemetry.h"
#if defined( POSIX )
#include "../../platform/posix/dynamic_library_provider.h"
#endif

#include <memory>
#include <string.h>
#if defined( POSIX )
#include <unistd.h>
#endif

#undef Sys_LoadModule

DEFINE_TESTSUITE( ModuleLoadTelemetryTestSuite )

namespace
{

#if defined( POSIX )
// A native fixture sits beside the host in a hand-assembled run directory and in
// bin/ in the installed layout. Resolving its path without calling the loader
// keeps the telemetry stream to exactly the events each test asserts.
const char *NativeFixturePath( const char *pFileName, char *pBuffer, size_t nBufferSize )
{
	static const char *const s_pDirectories[] = { ".", "./bin" };
	for ( const char *pDirectory : s_pDirectories )
	{
		Q_snprintf( pBuffer, nBufferSize, "%s/%s", pDirectory, pFileName );
		if ( access( pBuffer, F_OK ) == 0 )
			return pBuffer;
	}
	Q_snprintf( pBuffer, nBufferSize, "./%s", pFileName );
	return pBuffer;
}
#endif

ModuleLoadTelemetryEvent_t g_Events[64];
int g_nEventCount = 0;
CThreadFastMutex g_EventMutex;

void CaptureModuleLoadEvent(
	const ModuleLoadTelemetryEvent_t &event,
	void * )
{
	AUTO_LOCK( g_EventMutex );
	if ( g_nEventCount < ARRAYSIZE( g_Events ) )
		g_Events[g_nEventCount++] = event;
}

void CaptureOneModuleLoadEventAndUnregister(
	const ModuleLoadTelemetryEvent_t &event,
	void *pUserData )
{
	CaptureModuleLoadEvent( event, pUserData );
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );
}

void ResetEvents()
{
	AUTO_LOCK( g_EventMutex );
	memset( g_Events, 0, sizeof( g_Events ) );
	g_nEventCount = 0;
}

const ModuleLoadTelemetryEvent_t &Event( int index )
{
	Shipping_Assert( index >= 0 && index < g_nEventCount );
	return g_Events[index];
}

struct ConcurrentLoadRequest_t
{
	const char *m_pRequester;
	int m_nSourceLine;
};

uintp ConcurrentFailedLoad( void *pParameter )
{
	ConcurrentLoadRequest_t *pRequest =
		static_cast<ConcurrentLoadRequest_t *>( pParameter );
	CSysModule *pModule = Sys_LoadModuleWithContext(
		pRequest->m_pRequester,
		pRequest->m_nSourceLine,
		"phase_a_concurrent_fixture_that_does_not_exist_29D4" );
	Shipping_Assert( pModule == NULL );
	return 0;
}

} // namespace

DEFINE_TESTCASE( ModuleLoadTelemetryFailedLoad, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );
	CSysModule *pModule = Sys_LoadModuleWithContext(
		__FILE__, __LINE__, "phase_a_fixture_that_does_not_exist_7F31" );
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );

	Shipping_Assert( pModule == NULL );
	Shipping_Assert( g_nEventCount == 1 );
	Shipping_Assert( Event( 0 ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
	Shipping_Assert( !Event( 0 ).m_bSuccess );
	Shipping_Assert( Event( 0 ).m_nLoadId != 0 );
	Shipping_Assert( Event( 0 ).m_nProviderResult != 0 );
	Shipping_Assert( Q_stristr(
		Event( 0 ).m_szRequestingSubsystem,
		"moduleloadtelemetrytest.cpp" ) != NULL );
	Shipping_Assert( !Q_stricmp(
		Event( 0 ).m_szRequestedPath,
		"phase_a_fixture_that_does_not_exist_7F31" ) );
}

DEFINE_TESTCASE( ModuleLoadTelemetrySuccessfulLifecycle, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );
	CSysModule *pModule = NULL;
	void *pInterface = NULL;
	bool bLoaded = Sys_LoadInterface(
		"moduleloadfixture",
		"PhaseALoaderFixture001",
		&pModule,
		&pInterface );
	Shipping_Assert( bLoaded );
	Shipping_Assert( pModule != NULL );
	Shipping_Assert( pInterface != NULL );
	ThreadSleep( 1 );
	Sys_UnloadModule( pModule );
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );

	Shipping_Assert( g_nEventCount == 4 );
	Shipping_Assert( Event( 0 ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
	Shipping_Assert( Event( 0 ).m_bSuccess );
	Shipping_Assert( Event( 1 ).m_Operation == MODULE_LOAD_TELEMETRY_ENTRY_POINT );
	Shipping_Assert( Event( 1 ).m_bSuccess );
	Shipping_Assert( !Q_stricmp(
		Event( 1 ).m_szRequestedEntryPoint, "CreateInterface" ) );
	Shipping_Assert( Event( 2 ).m_Operation == MODULE_LOAD_TELEMETRY_INTERFACE );
	Shipping_Assert( Event( 2 ).m_bSuccess );
	Shipping_Assert( !Q_stricmp(
		Event( 2 ).m_szRequestedEntryPoint,
		"PhaseALoaderFixture001" ) );
	Shipping_Assert( Event( 3 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
	Shipping_Assert( Event( 3 ).m_bSuccess );
	Shipping_Assert( Event( 3 ).m_nLifetimeMicroseconds > 0 );
	for ( int i = 1; i < g_nEventCount; ++i )
		Shipping_Assert( Event( i ).m_nLoadId == Event( 0 ).m_nLoadId );
}

DEFINE_TESTCASE( ModuleLoadTelemetryPartialStartupRollback, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );
	CSysModule *pModule = NULL;
	void *pInterface = NULL;
	bool bLoaded = Sys_LoadInterface(
		"moduleloadfixture",
		"PhaseALoaderFixtureMissing001",
		&pModule,
		&pInterface );
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );

	Shipping_Assert( !bLoaded );
	Shipping_Assert( pInterface == NULL );
	Shipping_Assert( g_nEventCount == 4 );
	Shipping_Assert( Event( 0 ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
	Shipping_Assert( Event( 1 ).m_Operation == MODULE_LOAD_TELEMETRY_ENTRY_POINT );
	Shipping_Assert( Event( 2 ).m_Operation == MODULE_LOAD_TELEMETRY_INTERFACE );
	Shipping_Assert( !Event( 2 ).m_bSuccess );
	Shipping_Assert( Event( 3 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
	Shipping_Assert( Event( 3 ).m_bSuccess );
	for ( int i = 1; i < g_nEventCount; ++i )
		Shipping_Assert( Event( i ).m_nLoadId == Event( 0 ).m_nLoadId );
}

DEFINE_TESTCASE( ModuleLoadTelemetryDuplicateNativeHandle, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );
	CSysModule *pFirst = Sys_LoadModuleWithContext(
		__FILE__, __LINE__, "moduleloadfixture" );
	CSysModule *pSecond = Sys_LoadModuleWithContext(
		__FILE__, __LINE__, "moduleloadfixture" );
	Shipping_Assert( pFirst != NULL );
	Shipping_Assert( pSecond != NULL );
	const uint64 nFirstLoadId = Event( 0 ).m_nLoadId;
	const uint64 nSecondLoadId = Event( 1 ).m_nLoadId;
	Sys_UnloadModule( pSecond );
	Sys_UnloadModule( pFirst );
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );

	Shipping_Assert( g_nEventCount == 4 );
	Shipping_Assert( nFirstLoadId != nSecondLoadId );
	Shipping_Assert( Event( 2 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
	Shipping_Assert( Event( 2 ).m_nLoadId == nSecondLoadId );
	Shipping_Assert( Event( 3 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
	Shipping_Assert( Event( 3 ).m_nLoadId == nFirstLoadId );
}

DEFINE_TESTCASE( ModuleLoadTelemetryNestedRequestContext, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );
	{
		CScopedModuleLoadRequest outerRequest(
			"phase-a-outer-requester", 1701, "OuterInterface001" );
		{
			CScopedModuleLoadRequest innerRequest(
				"phase-a-inner-requester", 1702, "InnerInterface001" );
			CSysModule *pInner = Sys_LoadModule(
				"phase_a_nested_fixture_that_does_not_exist_492A" );
			Shipping_Assert( pInner == NULL );
		}
		CSysModule *pOuter = Sys_LoadModule(
			"phase_a_outer_fixture_that_does_not_exist_492B" );
		Shipping_Assert( pOuter == NULL );
	}
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );

	Shipping_Assert( g_nEventCount == 2 );
	Shipping_Assert( !Q_stricmp(
		Event( 0 ).m_szRequestingSubsystem, "phase-a-inner-requester" ) );
	Shipping_Assert( Event( 0 ).m_nSourceLine == 1702 );
	Shipping_Assert( !Q_stricmp(
		Event( 0 ).m_szRequestedEntryPoint, "InnerInterface001" ) );
	Shipping_Assert( !Q_stricmp(
		Event( 1 ).m_szRequestingSubsystem, "phase-a-outer-requester" ) );
	Shipping_Assert( Event( 1 ).m_nSourceLine == 1701 );
	Shipping_Assert( !Q_stricmp(
		Event( 1 ).m_szRequestedEntryPoint, "OuterInterface001" ) );
}

DEFINE_TESTCASE( ModuleLoadTelemetryConcurrentRequests, ModuleLoadTelemetryTestSuite )
{
	ConcurrentLoadRequest_t requests[] = {
		{ "phase-a-thread-0", 1800 },
		{ "phase-a-thread-1", 1801 },
		{ "phase-a-thread-2", 1802 },
		{ "phase-a-thread-3", 1803 },
	};
	ThreadHandle_t threads[ARRAYSIZE( requests )];

	ResetEvents();
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );
	for ( int i = 0; i < ARRAYSIZE( requests ); ++i )
		threads[i] = CreateSimpleThread( ConcurrentFailedLoad, &requests[i] );
	for ( int i = 0; i < ARRAYSIZE( requests ); ++i )
	{
		Shipping_Assert( threads[i] != 0 );
		ThreadJoin( threads[i] );
		ReleaseThreadHandle( threads[i] );
	}
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );

	Shipping_Assert( g_nEventCount == ARRAYSIZE( requests ) );
	for ( int i = 0; i < g_nEventCount; ++i )
	{
		Shipping_Assert( Event( i ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
		Shipping_Assert( !Event( i ).m_bSuccess );
		for ( int j = i + 1; j < g_nEventCount; ++j )
			Shipping_Assert( Event( i ).m_nLoadId != Event( j ).m_nLoadId );

		bool bFoundRequester = false;
		for ( int j = 0; j < ARRAYSIZE( requests ); ++j )
		{
			if ( !Q_stricmp( Event( i ).m_szRequestingSubsystem,
				requests[j].m_pRequester ) )
			{
				Shipping_Assert(
					Event( i ).m_nSourceLine == requests[j].m_nSourceLine );
				bFoundRequester = true;
				break;
			}
		}
		Shipping_Assert( bFoundRequester );
	}
}

DEFINE_TESTCASE( ModuleLoadTelemetrySinkCanUnregisterItself, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	Sys_SetModuleLoadTelemetrySink(
		CaptureOneModuleLoadEventAndUnregister, NULL );
	CSysModule *pFirst = Sys_LoadModuleWithContext(
		__FILE__, __LINE__, "phase_a_reentrant_fixture_missing_11A0" );
	CSysModule *pSecond = Sys_LoadModuleWithContext(
		__FILE__, __LINE__, "phase_a_reentrant_fixture_missing_11A1" );

	Shipping_Assert( pFirst == NULL );
	Shipping_Assert( pSecond == NULL );
	Shipping_Assert( g_nEventCount == 1 );
}

DEFINE_TESTCASE( ModuleLoadTelemetryNativeLoaderLifecycle, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );

#if defined( POSIX )
	char szFixture[256];
	void *pModule = dlopen(
	    NativeFixturePath( "libmoduleloadfixture.so", szFixture, sizeof( szFixture ) ), RTLD_NOW );
	Shipping_Assert( pModule != NULL );
	void *pEntryPoint = dlsym( pModule, "PhaseALoaderFixtureSymbol" );
	Shipping_Assert( pEntryPoint != NULL );
	const int nUnloadResult = dlclose( pModule );
	Shipping_Assert( nUnloadResult == 0 );
#elif defined( _WIN32 )
	HMODULE pModule = LoadLibraryA( "moduleloadfixture.dll" );
	Shipping_Assert( pModule != NULL );
	FARPROC pEntryPoint = GetProcAddress(
		pModule, "PhaseALoaderFixtureSymbol" );
	Shipping_Assert( pEntryPoint != NULL );
	const BOOL bUnloadResult = FreeLibrary( pModule );
	Shipping_Assert( bUnloadResult != FALSE );
#endif

	Sys_SetModuleLoadTelemetrySink( NULL, NULL );
	Shipping_Assert( g_nEventCount == 3 );
	Shipping_Assert( Event( 0 ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
	Shipping_Assert( Event( 0 ).m_bSuccess );
	Shipping_Assert( Q_stristr(
		Event( 0 ).m_szResolvedPath, "moduleloadfixture" ) != NULL );
	Shipping_Assert( Event( 1 ).m_Operation == MODULE_LOAD_TELEMETRY_ENTRY_POINT );
	Shipping_Assert( Event( 1 ).m_bSuccess );
	Shipping_Assert( !Q_stricmp(
		Event( 1 ).m_szRequestedEntryPoint,
		"PhaseALoaderFixtureSymbol" ) );
	Shipping_Assert( Event( 2 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
	Shipping_Assert( Event( 2 ).m_bSuccess );
	Shipping_Assert( Event( 0 ).m_nLoadId == Event( 1 ).m_nLoadId );
	Shipping_Assert( Event( 0 ).m_nLoadId == Event( 2 ).m_nLoadId );
}

#if defined( POSIX )
// R07-SYS: a platform loader provider's events reach the same stream, with the
// caller's request context, distinct IDs for duplicate loads, and correlated
// entry-point and unload records.
DEFINE_TESTCASE( ModuleLoadTelemetryPlatformProvider, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	CModuleLoadTelemetryObserver observer;
	std::unique_ptr<platform::IDynamicLibraryLoader> loader =
	    platform::CreatePosixDynamicLibraryLoader( observer );
	char szFixture[256];
	NativeFixturePath( "libmoduleloadfixture.so", szFixture, sizeof( szFixture ) );
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );
	{
		CScopedModuleLoadRequest request( "PlatformProviderTest", 4242 );
		platform::IDynamicLibrary *pFirst = loader->Load( szFixture, NULL );
		platform::IDynamicLibrary *pSecond = loader->Load( szFixture, NULL );
		Shipping_Assert( pFirst != NULL && pSecond != NULL && pFirst != pSecond );
		Shipping_Assert( pFirst->FindSymbol( "PhaseALoaderFixtureSymbol", NULL ) != NULL );
		Shipping_Assert( loader->Load( "./phase_a_platform_missing_7F1C.so", NULL ) == NULL );
		loader->Unload( pSecond );
		loader->Unload( pFirst );
	}
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );

	Shipping_Assert( g_nEventCount == 6 );
	Shipping_Assert( Event( 0 ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
	Shipping_Assert( Event( 0 ).m_bSuccess );
	Shipping_Assert( !Q_strcmp( Event( 0 ).m_szRequestingSubsystem, "PlatformProviderTest" ) );
	Shipping_Assert( Event( 0 ).m_nSourceLine == 4242 );
	Shipping_Assert( Event( 0 ).m_szResolvedPath[0] == '/' );
	Shipping_Assert( Q_stristr( Event( 0 ).m_szResolvedPath, "moduleloadfixture" ) != NULL );
	Shipping_Assert( Event( 1 ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
	Shipping_Assert( Event( 1 ).m_bSuccess );
	Shipping_Assert( Event( 1 ).m_nLoadId != Event( 0 ).m_nLoadId );
	Shipping_Assert( Event( 2 ).m_Operation == MODULE_LOAD_TELEMETRY_ENTRY_POINT );
	Shipping_Assert( Event( 2 ).m_bSuccess );
	Shipping_Assert( Event( 2 ).m_nLoadId == Event( 0 ).m_nLoadId );
	Shipping_Assert(
	    !Q_strcmp( Event( 2 ).m_szRequestedEntryPoint, "PhaseALoaderFixtureSymbol" ) );
	Shipping_Assert( Event( 3 ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
	Shipping_Assert( !Event( 3 ).m_bSuccess );
	Shipping_Assert( Event( 3 ).m_nProviderResult != 0 );
	Shipping_Assert( !Q_strcmp( Event( 3 ).m_szProviderError, "not found" ) );
	Shipping_Assert( Event( 4 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
	Shipping_Assert( Event( 4 ).m_nLoadId == Event( 1 ).m_nLoadId );
	Shipping_Assert( Event( 5 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
	Shipping_Assert( Event( 5 ).m_nLoadId == Event( 0 ).m_nLoadId );
	Shipping_Assert( loader->LiveLibraryCount() == 0 );
}
#endif

DEFINE_TESTCASE( ModuleLoadTelemetryFrozenLegacyAbi, ModuleLoadTelemetryTestSuite )
{
	ResetEvents();
	Sys_SetModuleLoadTelemetrySink( CaptureModuleLoadEvent, NULL );
#if defined( POSIX )
	char szFixture[256];
	void *pClientModule = dlopen(
	    NativeFixturePath( "liblegacymoduleclientfixture.so", szFixture, sizeof( szFixture ) ),
	    RTLD_NOW );
	Shipping_Assert( pClientModule != NULL );
	PhaseALegacyLoaderClientProbeFn pProbe =
		reinterpret_cast<PhaseALegacyLoaderClientProbeFn>( dlsym(
			pClientModule, PHASE_A_LEGACY_LOADER_CLIENT_PROBE ) );
	Shipping_Assert( pProbe != NULL );
	Shipping_Assert( pProbe() != 0 );
	Shipping_Assert( dlclose( pClientModule ) == 0 );
#elif defined( _WIN32 )
	HMODULE pClientModule = LoadLibraryA(
		"legacymoduleclientfixture.dll" );
	Shipping_Assert( pClientModule != NULL );
	PhaseALegacyLoaderClientProbeFn pProbe =
		reinterpret_cast<PhaseALegacyLoaderClientProbeFn>( GetProcAddress(
			pClientModule, PHASE_A_LEGACY_LOADER_CLIENT_PROBE ) );
	Shipping_Assert( pProbe != NULL );
	Shipping_Assert( pProbe() != 0 );
	Shipping_Assert( FreeLibrary( pClientModule ) != FALSE );
#endif
	Sys_SetModuleLoadTelemetrySink( NULL, NULL );

	Shipping_Assert( g_nEventCount == 6 );
	Shipping_Assert( Event( 2 ).m_Operation == MODULE_LOAD_TELEMETRY_LOAD );
	Shipping_Assert( Event( 2 ).m_bSuccess );
	Shipping_Assert( !Q_stricmp(
		Event( 2 ).m_szRequestingSubsystem, "<legacy ABI caller>" ) );
	Shipping_Assert( Event( 3 ).m_Operation == MODULE_LOAD_TELEMETRY_ENTRY_POINT );
	Shipping_Assert( Event( 4 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
	Shipping_Assert( Event( 4 ).m_bSuccess );
	Shipping_Assert( Event( 2 ).m_nLoadId == Event( 3 ).m_nLoadId );
	Shipping_Assert( Event( 2 ).m_nLoadId == Event( 4 ).m_nLoadId );
	Shipping_Assert( Event( 5 ).m_Operation == MODULE_LOAD_TELEMETRY_UNLOAD );
}
