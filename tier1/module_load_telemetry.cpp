//========= Copyright Valve Corporation, All rights reserved. ============//
//
// RFC 0001 Phase A dynamic-module telemetry.
//
//=============================================================================//

#include "tier1/module_load_telemetry_internal.h"

#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "tier0/threadtools.h"
#include "tier1/strtools.h"

#include <string.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{

struct ModuleLoadRecord_t
{
	ModuleLoadRecord_t *m_pNext;
	CSysModule *m_pModule;
	uint64 m_nLoadId;
	uint64 m_nStartedAtMicroseconds;
	int m_nSourceLine;
	char m_szRequestingSubsystem[260];
	char m_szRequestedPath[2048];
	char m_szResolvedPath[2048];
};

CThreadFastMutex g_ModuleLoadTelemetryMutex;
CThreadLocalPtr<CScopedModuleLoadRequest> g_pModuleLoadRequest;
ModuleLoadRecord_t *g_pModuleLoadRecords = NULL;
ModuleLoadTelemetrySinkFn g_pModuleLoadTelemetrySink = NULL;
void *g_pModuleLoadTelemetryUserData = NULL;
uint64 g_nNextModuleLoadId = 1;

const char *SafeString( const char *pValue )
{
	return pValue ? pValue : "";
}

void CopyString( char *pDestination, int nDestinationSize, const char *pSource )
{
	Q_strncpy( pDestination, SafeString( pSource ), nDestinationSize );
}

const CScopedModuleLoadRequest *CurrentRequest()
{
	return g_pModuleLoadRequest;
}

void FillRequestContext( ModuleLoadTelemetryEvent_t &event )
{
	const CScopedModuleLoadRequest *pRequest = CurrentRequest();
	if ( pRequest )
	{
		CopyString( event.m_szRequestingSubsystem,
			sizeof( event.m_szRequestingSubsystem ),
			pRequest->GetRequestingSubsystem() );
		event.m_nSourceLine = pRequest->GetSourceLine();
		CopyString( event.m_szRequestedEntryPoint,
			sizeof( event.m_szRequestedEntryPoint ),
			pRequest->GetRequestedInterface() );
	}
	else
	{
		CopyString( event.m_szRequestingSubsystem,
			sizeof( event.m_szRequestingSubsystem ),
			"<legacy ABI caller>" );
		event.m_nSourceLine = 0;
	}
}

void FillRecordContext(
	ModuleLoadTelemetryEvent_t &event,
	const ModuleLoadRecord_t &record )
{
	event.m_nLoadId = record.m_nLoadId;
	event.m_nSourceLine = record.m_nSourceLine;
	CopyString( event.m_szRequestingSubsystem,
		sizeof( event.m_szRequestingSubsystem ),
		record.m_szRequestingSubsystem );
	CopyString( event.m_szRequestedPath,
		sizeof( event.m_szRequestedPath ),
		record.m_szRequestedPath );
	CopyString( event.m_szResolvedPath,
		sizeof( event.m_szResolvedPath ),
		record.m_szResolvedPath );
}

ModuleLoadTelemetryEvent_t EmptyEvent( ModuleLoadTelemetryOperation_t operation )
{
	ModuleLoadTelemetryEvent_t event;
	memset( &event, 0, sizeof( event ) );
	event.m_Operation = operation;
	return event;
}

ModuleLoadRecord_t *FindRecord( CSysModule *pModule )
{
	for ( ModuleLoadRecord_t *pRecord = g_pModuleLoadRecords;
		pRecord;
		pRecord = pRecord->m_pNext )
	{
		if ( pRecord->m_pModule == pModule )
			return pRecord;
	}
	return NULL;
}

void EmitEvent( const ModuleLoadTelemetryEvent_t &event )
{
	ModuleLoadTelemetrySinkFn pSink;
	void *pUserData;
	{
		AUTO_LOCK( g_ModuleLoadTelemetryMutex );
		pSink = g_pModuleLoadTelemetrySink;
		pUserData = g_pModuleLoadTelemetryUserData;
	}

	if ( CommandLine() && CommandLine()->FindParm( "-moduleloadtelemetry" ) )
	{
		Msg( "ModuleLoadTelemetry: op=%d id=%llu requester=%s:%d requested=%s resolved=%s entry=%s success=%d lifetime_us=%llu provider_result=%d error=%s\n",
			(int)event.m_Operation,
			(unsigned long long)event.m_nLoadId,
			event.m_szRequestingSubsystem,
			event.m_nSourceLine,
			event.m_szRequestedPath,
			event.m_szResolvedPath,
			event.m_szRequestedEntryPoint,
			event.m_bSuccess ? 1 : 0,
			(unsigned long long)event.m_nLifetimeMicroseconds,
			event.m_nProviderResult,
			event.m_szProviderError );
	}

	if ( pSink )
		pSink( event, pUserData );
}

} // namespace

void Sys_SetModuleLoadTelemetrySink(
	ModuleLoadTelemetrySinkFn pSink,
	void *pUserData )
{
	AUTO_LOCK( g_ModuleLoadTelemetryMutex );
	g_pModuleLoadTelemetrySink = pSink;
	g_pModuleLoadTelemetryUserData = pUserData;
}

CScopedModuleLoadRequest::CScopedModuleLoadRequest(
	const char *pRequestingSubsystem,
	int nSourceLine,
	const char *pRequestedInterface )
	: m_pPrevious( g_pModuleLoadRequest )
	, m_pRequestingSubsystem( pRequestingSubsystem )
	, m_pRequestedInterface( pRequestedInterface )
	, m_nSourceLine( nSourceLine )
{
	g_pModuleLoadRequest = this;
}

CScopedModuleLoadRequest::~CScopedModuleLoadRequest()
{
	Assert( g_pModuleLoadRequest == this );
	g_pModuleLoadRequest = m_pPrevious;
}

void Sys_RecordModuleLoad(
	CSysModule *pModule,
	const char *pRequestedPath,
	const char *pResolvedPath,
	int nProviderResult,
	const char *pProviderError )
{
	ModuleLoadTelemetryEvent_t event = EmptyEvent( MODULE_LOAD_TELEMETRY_LOAD );
	event.m_bSuccess = pModule != NULL;
	event.m_nProviderResult = nProviderResult;
	FillRequestContext( event );
	CopyString( event.m_szRequestedPath, sizeof( event.m_szRequestedPath ), pRequestedPath );
	CopyString( event.m_szResolvedPath, sizeof( event.m_szResolvedPath ), pResolvedPath );
	CopyString( event.m_szProviderError, sizeof( event.m_szProviderError ), pProviderError );

	ModuleLoadRecord_t *pRecord = NULL;
	if ( pModule )
	{
		pRecord = new ModuleLoadRecord_t;
		memset( pRecord, 0, sizeof( *pRecord ) );
		pRecord->m_pModule = pModule;
		pRecord->m_nStartedAtMicroseconds = Plat_USTime();
		pRecord->m_nSourceLine = event.m_nSourceLine;
		CopyString( pRecord->m_szRequestingSubsystem,
			sizeof( pRecord->m_szRequestingSubsystem ),
			event.m_szRequestingSubsystem );
		CopyString( pRecord->m_szRequestedPath,
			sizeof( pRecord->m_szRequestedPath ),
			pRequestedPath );
		CopyString( pRecord->m_szResolvedPath,
			sizeof( pRecord->m_szResolvedPath ),
			pResolvedPath );
	}

	{
		AUTO_LOCK( g_ModuleLoadTelemetryMutex );
		event.m_nLoadId = g_nNextModuleLoadId++;
		if ( pRecord )
		{
			pRecord->m_nLoadId = event.m_nLoadId;
			pRecord->m_pNext = g_pModuleLoadRecords;
			g_pModuleLoadRecords = pRecord;
		}
	}

	EmitEvent( event );
}

void Sys_RecordModuleEntryPoint(
	CSysModule *pModule,
	const char *pRequestedPath,
	const char *pEntryPoint,
	bool bSuccess,
	int nProviderResult,
	const char *pProviderError )
{
	ModuleLoadTelemetryEvent_t event = EmptyEvent( MODULE_LOAD_TELEMETRY_ENTRY_POINT );
	event.m_bSuccess = bSuccess;
	event.m_nProviderResult = nProviderResult;
	CopyString( event.m_szRequestedEntryPoint,
		sizeof( event.m_szRequestedEntryPoint ),
		pEntryPoint );
	CopyString( event.m_szProviderError,
		sizeof( event.m_szProviderError ),
		pProviderError );

	{
		AUTO_LOCK( g_ModuleLoadTelemetryMutex );
		ModuleLoadRecord_t *pRecord = FindRecord( pModule );
		if ( pRecord )
			FillRecordContext( event, *pRecord );
		else
		{
			FillRequestContext( event );
			CopyString( event.m_szRequestedPath,
				sizeof( event.m_szRequestedPath ),
				pRequestedPath );
		}
	}

	EmitEvent( event );
}

void Sys_RecordModuleInterface(
	CSysModule *pModule,
	const char *pInterfaceName,
	bool bSuccess )
{
	ModuleLoadTelemetryEvent_t event = EmptyEvent( MODULE_LOAD_TELEMETRY_INTERFACE );
	event.m_bSuccess = bSuccess;
	CopyString( event.m_szRequestedEntryPoint,
		sizeof( event.m_szRequestedEntryPoint ),
		pInterfaceName );
	{
		AUTO_LOCK( g_ModuleLoadTelemetryMutex );
		ModuleLoadRecord_t *pRecord = FindRecord( pModule );
		if ( pRecord )
			FillRecordContext( event, *pRecord );
		else
			FillRequestContext( event );
	}
	EmitEvent( event );
}

void Sys_RecordModuleUnload(
	CSysModule *pModule,
	bool bSuccess,
	int nProviderResult,
	const char *pProviderError )
{
	ModuleLoadTelemetryEvent_t event = EmptyEvent( MODULE_LOAD_TELEMETRY_UNLOAD );
	event.m_bSuccess = bSuccess;
	event.m_nProviderResult = nProviderResult;
	CopyString( event.m_szProviderError,
		sizeof( event.m_szProviderError ),
		pProviderError );

	ModuleLoadRecord_t *pRemoved = NULL;
	{
		AUTO_LOCK( g_ModuleLoadTelemetryMutex );
		ModuleLoadRecord_t **ppRecord = &g_pModuleLoadRecords;
		while ( *ppRecord && ( *ppRecord )->m_pModule != pModule )
			ppRecord = &( *ppRecord )->m_pNext;
		if ( *ppRecord )
		{
			ModuleLoadRecord_t *pRecord = *ppRecord;
			FillRecordContext( event, *pRecord );
			event.m_nLifetimeMicroseconds =
				Plat_USTime() - pRecord->m_nStartedAtMicroseconds;
			if ( bSuccess )
			{
				*ppRecord = pRecord->m_pNext;
				pRemoved = pRecord;
			}
		}
		else
		{
			FillRequestContext( event );
		}
	}

	EmitEvent( event );
	delete pRemoved;
}
