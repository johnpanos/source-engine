//========= Copyright Valve Corporation, All rights reserved. ============//
//
// RFC 0001 Phase A loader telemetry tests.
//
//=============================================================================//

#include "unitlib/unitlib.h"
#include "tier0/dbg.h"
#include "tier0/threadtools.h"
#include "tier1/interface.h"
#include "tier1/module_load_telemetry.h"
#include "tier1/strtools.h"

#include <string.h>

#undef Sys_LoadModule

DEFINE_TESTSUITE( ModuleLoadTelemetryTestSuite )

namespace
{

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
