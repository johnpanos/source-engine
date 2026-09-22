//========= Copyright Valve Corporation, All rights reserved. ============//
//
// RFC 0001 Phase A loader telemetry tests.
//
//=============================================================================//

#include "unitlib/unitlib.h"
#include "tier0/dbg.h"
#include "tier1/interface.h"
#include "tier1/module_load_telemetry.h"
#include "tier1/strtools.h"

#include <string.h>

DEFINE_TESTSUITE( ModuleLoadTelemetryTestSuite )

namespace
{

ModuleLoadTelemetryEvent_t g_Events[32];
int g_nEventCount = 0;

void CaptureModuleLoadEvent(
	const ModuleLoadTelemetryEvent_t &event,
	void * )
{
	if ( g_nEventCount < ARRAYSIZE( g_Events ) )
		g_Events[g_nEventCount++] = event;
}

void ResetEvents()
{
	memset( g_Events, 0, sizeof( g_Events ) );
	g_nEventCount = 0;
}

const ModuleLoadTelemetryEvent_t &Event( int index )
{
	Shipping_Assert( index >= 0 && index < g_nEventCount );
	return g_Events[index];
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
