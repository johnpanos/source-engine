//========= Copyright Valve Corporation, All rights reserved. ============//
//
// RFC 0001 Phase A dynamic-module telemetry.
//
//=============================================================================//

#include "tier0/module_load_telemetry_internal.h"

#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#define NATIVE_MODULE_LOAD_TELEMETRY_IMPLEMENTATION
#include "tier0/native_module_load_telemetry.h"
#include "tier0/threadtools.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined( POSIX )
#include <limits.h>
#if defined( PLATFORM_GLIBC )
#include <link.h>
#endif
#endif

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
bool g_bTelemetryLogEnabled = false;

const char *SafeString( const char *pValue )
{
	return pValue ? pValue : "";
}

void CopyString( char *pDestination, int nDestinationSize, const char *pSource )
{
	if ( nDestinationSize <= 0 )
		return;
	strncpy( pDestination, SafeString( pSource ), nDestinationSize );
	pDestination[nDestinationSize - 1] = '\0';
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
		{
			AUTO_LOCK( g_ModuleLoadTelemetryMutex );
			g_bTelemetryLogEnabled = true;
		}
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

class CModuleLoadTelemetryShutdownReporter
{
public:
	~CModuleLoadTelemetryShutdownReporter()
	{
		ModuleLoadRecord_t *pRecords;
		{
			AUTO_LOCK( g_ModuleLoadTelemetryMutex );
			pRecords = g_pModuleLoadRecords;
			g_pModuleLoadRecords = NULL;
			g_pModuleLoadTelemetrySink = NULL;
			g_pModuleLoadTelemetryUserData = NULL;
		}

		while ( pRecords )
		{
			ModuleLoadRecord_t *pRecord = pRecords;
			pRecords = pRecord->m_pNext;
			if ( g_bTelemetryLogEnabled )
			{
				fprintf( stderr,
					"ModuleLoadTelemetry: op=%d id=%llu requester=%s:%d requested=%s resolved=%s entry= success=0 lifetime_us=%llu provider_result=-1 error=module still loaded at process shutdown\n",
					(int)MODULE_LOAD_TELEMETRY_UNLOAD,
					(unsigned long long)pRecord->m_nLoadId,
					pRecord->m_szRequestingSubsystem,
					pRecord->m_nSourceLine,
					pRecord->m_szRequestedPath,
					pRecord->m_szResolvedPath,
					(unsigned long long)( Plat_USTime() -
						pRecord->m_nStartedAtMicroseconds ) );
			}
			delete pRecord;
		}
	}
};

CModuleLoadTelemetryShutdownReporter g_ModuleLoadTelemetryShutdownReporter;

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

namespace
{

#if defined( POSIX )
void ResolveNativeModulePath(
	void *pModule,
	const char *pRequestedPath,
	char *pResolvedPath,
	int nResolvedPathSize )
{
	CopyString( pResolvedPath, nResolvedPathSize, pRequestedPath );
	char szCanonicalPath[PATH_MAX];
	if ( pRequestedPath && realpath( pRequestedPath, szCanonicalPath ) )
		CopyString( pResolvedPath, nResolvedPathSize, szCanonicalPath );
#if defined( PLATFORM_GLIBC )
	link_map *pMap = NULL;
	if ( pModule && dlinfo( pModule, RTLD_DI_LINKMAP, &pMap ) == 0 &&
		pMap && pMap->l_name && pMap->l_name[0] )
	{
		if ( realpath( pMap->l_name, szCanonicalPath ) )
			CopyString( pResolvedPath, nResolvedPathSize, szCanonicalPath );
		else
			CopyString( pResolvedPath, nResolvedPathSize, pMap->l_name );
	}
#endif
}
#elif defined( _WIN32 )
void WideToUtf8( const wchar_t *pSource, char *pDestination, int nDestinationSize )
{
	if ( !pSource || nDestinationSize <= 0 )
	{
		if ( nDestinationSize > 0 )
			pDestination[0] = '\0';
		return;
	}
	if ( !WideCharToMultiByte( CP_UTF8, 0, pSource, -1,
		pDestination, nDestinationSize, NULL, NULL ) )
	{
		pDestination[0] = '\0';
	}
}

void WindowsErrorText( DWORD nError, char *pError, int nErrorSize )
{
	if ( nError == ERROR_SUCCESS )
	{
		CopyString( pError, nErrorSize, "" );
		return;
	}
	if ( !FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, nError, 0, pError, nErrorSize, NULL ) )
	{
		snprintf( pError, nErrorSize, "Win32 error %lu",
			(unsigned long)nError );
	}
}

void ResolveNativeModulePath(
	HMODULE hModule,
	const char *pRequestedPath,
	char *pResolvedPath,
	int nResolvedPathSize )
{
	CopyString( pResolvedPath, nResolvedPathSize, pRequestedPath );
	if ( hModule && GetModuleFileNameA(
		hModule, pResolvedPath, nResolvedPathSize ) == 0 )
	{
		CopyString( pResolvedPath, nResolvedPathSize, pRequestedPath );
	}
}
#endif

} // namespace

#if defined( POSIX )
void *ModuleLoadTelemetry_dlopen(
	const char *pPath,
	int nMode,
	const char *pRequester,
	int nSourceLine )
{
	CScopedModuleLoadRequest request( pRequester, nSourceLine );
	dlerror();
	errno = 0;
	void *pModule = dlopen( pPath, nMode );
	const int nProviderResult = pModule ? 0 : ( errno ? errno : -1 );
	const char *pProviderError = pModule ? "" : dlerror();
	char szResolvedPath[2048];
	ResolveNativeModulePath(
		pModule, pPath, szResolvedPath, sizeof( szResolvedPath ) );
	Sys_RecordModuleLoad(
		reinterpret_cast<CSysModule *>( pModule ),
		pPath, szResolvedPath, nProviderResult, pProviderError );
	return pModule;
}

void *ModuleLoadTelemetry_dlsym(
	void *pModule,
	const char *pEntryPoint,
	const char *pRequester,
	int nSourceLine )
{
	CScopedModuleLoadRequest request( pRequester, nSourceLine );
	dlerror();
	errno = 0;
	void *pResult = dlsym( pModule, pEntryPoint );
	const char *pProviderError = dlerror();
	const bool bSuccess = pProviderError == NULL;
	Sys_RecordModuleEntryPoint(
		reinterpret_cast<CSysModule *>( pModule ), NULL,
		pEntryPoint, bSuccess, bSuccess ? 0 : ( errno ? errno : -1 ),
		pProviderError );
	return pResult;
}

int ModuleLoadTelemetry_dlclose(
	void *pModule,
	const char *pRequester,
	int nSourceLine )
{
	CScopedModuleLoadRequest request( pRequester, nSourceLine );
	dlerror();
	errno = 0;
	const int nResult = dlclose( pModule );
	const char *pProviderError = nResult == 0 ? "" : dlerror();
	Sys_RecordModuleUnload(
		reinterpret_cast<CSysModule *>( pModule ),
		nResult == 0, nResult == 0 ? 0 : ( errno ? errno : nResult ),
		pProviderError );
	return nResult;
}
#elif defined( _WIN32 )
HMODULE ModuleLoadTelemetry_LoadLibraryA(
	LPCSTR pPath,
	const char *pRequester,
	int nSourceLine )
{
	return ModuleLoadTelemetry_LoadLibraryExA(
		pPath, NULL, 0, pRequester, nSourceLine );
}

HMODULE ModuleLoadTelemetry_LoadLibraryW(
	LPCWSTR pPath,
	const char *pRequester,
	int nSourceLine )
{
	return ModuleLoadTelemetry_LoadLibraryExW(
		pPath, NULL, 0, pRequester, nSourceLine );
}

HMODULE ModuleLoadTelemetry_LoadLibraryExA(
	LPCSTR pPath,
	HANDLE hFile,
	DWORD nFlags,
	const char *pRequester,
	int nSourceLine )
{
	CScopedModuleLoadRequest request( pRequester, nSourceLine );
	SetLastError( ERROR_SUCCESS );
	HMODULE hModule = LoadLibraryExA( pPath, hFile, nFlags );
	const DWORD nProviderResult = hModule ? ERROR_SUCCESS : GetLastError();
	char szResolvedPath[2048];
	char szProviderError[512];
	ResolveNativeModulePath(
		hModule, pPath, szResolvedPath, sizeof( szResolvedPath ) );
	WindowsErrorText(
		nProviderResult, szProviderError, sizeof( szProviderError ) );
	Sys_RecordModuleLoad(
		reinterpret_cast<CSysModule *>( hModule ),
		pPath, szResolvedPath, (int)nProviderResult, szProviderError );
	return hModule;
}

HMODULE ModuleLoadTelemetry_LoadLibraryExW(
	LPCWSTR pPath,
	HANDLE hFile,
	DWORD nFlags,
	const char *pRequester,
	int nSourceLine )
{
	char szRequestedPath[2048];
	WideToUtf8( pPath, szRequestedPath, sizeof( szRequestedPath ) );
	CScopedModuleLoadRequest request( pRequester, nSourceLine );
	SetLastError( ERROR_SUCCESS );
	HMODULE hModule = LoadLibraryExW( pPath, hFile, nFlags );
	const DWORD nProviderResult = hModule ? ERROR_SUCCESS : GetLastError();
	char szResolvedPath[2048];
	char szProviderError[512];
	ResolveNativeModulePath( hModule, szRequestedPath,
		szResolvedPath, sizeof( szResolvedPath ) );
	WindowsErrorText(
		nProviderResult, szProviderError, sizeof( szProviderError ) );
	Sys_RecordModuleLoad(
		reinterpret_cast<CSysModule *>( hModule ),
		szRequestedPath, szResolvedPath,
		(int)nProviderResult, szProviderError );
	return hModule;
}

FARPROC ModuleLoadTelemetry_GetProcAddress(
	HMODULE hModule,
	LPCSTR pEntryPoint,
	const char *pRequester,
	int nSourceLine )
{
	CScopedModuleLoadRequest request( pRequester, nSourceLine );
	SetLastError( ERROR_SUCCESS );
	FARPROC pResult = GetProcAddress( hModule, pEntryPoint );
	const DWORD nProviderResult = pResult ? ERROR_SUCCESS : GetLastError();
	char szProviderError[512];
	WindowsErrorText(
		nProviderResult, szProviderError, sizeof( szProviderError ) );
	Sys_RecordModuleEntryPoint(
		reinterpret_cast<CSysModule *>( hModule ), NULL,
		pEntryPoint, pResult != NULL, (int)nProviderResult, szProviderError );
	return pResult;
}

BOOL ModuleLoadTelemetry_FreeLibrary(
	HMODULE hModule,
	const char *pRequester,
	int nSourceLine )
{
	CScopedModuleLoadRequest request( pRequester, nSourceLine );
	SetLastError( ERROR_SUCCESS );
	const BOOL bResult = FreeLibrary( hModule );
	const DWORD nProviderResult = bResult ? ERROR_SUCCESS : GetLastError();
	char szProviderError[512];
	WindowsErrorText(
		nProviderResult, szProviderError, sizeof( szProviderError ) );
	Sys_RecordModuleUnload(
		reinterpret_cast<CSysModule *>( hModule ), bResult != FALSE,
		(int)nProviderResult, szProviderError );
	return bResult;
}
#endif
