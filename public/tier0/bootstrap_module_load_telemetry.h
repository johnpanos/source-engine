//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Self-contained native-loader telemetry for executables which bootstrap
// Tier 0 itself. Include in exactly one translation unit after OS loader
// declarations, then initialize it before the first module load.
//
//=============================================================================//

#ifndef TIER0_BOOTSTRAP_MODULE_LOAD_TELEMETRY_H
#define TIER0_BOOTSTRAP_MODULE_LOAD_TELEMETRY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined( POSIX )
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#if defined( __GLIBC__ )
#include <link.h>
#endif
#endif

struct BootstrapModuleLoadRecord_t
{
	BootstrapModuleLoadRecord_t *m_pNext;
	void *m_pModule;
	unsigned long long m_nLoadId;
	unsigned long long m_nStartedAtMicroseconds;
	const char *m_pRequester;
	int m_nSourceLine;
	char m_szRequestedPath[2048];
	char m_szResolvedPath[2048];
};

static BootstrapModuleLoadRecord_t *g_pBootstrapModuleLoads = NULL;
static unsigned long long g_nNextBootstrapModuleLoadId = 1;
static bool g_bBootstrapModuleLoadTelemetryEnabled = false;

static void BootstrapModuleLoadCopy(
	char *pDestination, int nDestinationSize, const char *pSource )
{
	if ( nDestinationSize <= 0 )
		return;
	strncpy( pDestination, pSource ? pSource : "", nDestinationSize );
	pDestination[nDestinationSize - 1] = '\0';
}

static unsigned long long BootstrapModuleLoadTime()
{
#if defined( _WIN32 )
	LARGE_INTEGER counter;
	LARGE_INTEGER frequency;
	QueryPerformanceCounter( &counter );
	QueryPerformanceFrequency( &frequency );
	return (unsigned long long)(
		counter.QuadPart * 1000000ULL / frequency.QuadPart );
#else
	timespec value;
	clock_gettime( CLOCK_MONOTONIC, &value );
	return (unsigned long long)value.tv_sec * 1000000ULL +
		(unsigned long long)value.tv_nsec / 1000ULL;
#endif
}

static void BootstrapModuleLoadEmit(
	int nOperation,
	unsigned long long nLoadId,
	const char *pRequester,
	int nSourceLine,
	const char *pRequestedPath,
	const char *pResolvedPath,
	const char *pEntryPoint,
	bool bSuccess,
	unsigned long long nLifetime,
	int nProviderResult,
	const char *pProviderError )
{
	if ( !g_bBootstrapModuleLoadTelemetryEnabled )
		return;
	fprintf( stderr,
		"ModuleLoadTelemetry: op=%d id=%llu requester=%s:%d requested=%s resolved=%s entry=%s success=%d lifetime_us=%llu provider_result=%d error=%s\n",
		nOperation, nLoadId, pRequester ? pRequester : "", nSourceLine,
		pRequestedPath ? pRequestedPath : "",
		pResolvedPath ? pResolvedPath : "",
		pEntryPoint ? pEntryPoint : "", bSuccess ? 1 : 0,
		nLifetime, nProviderResult, pProviderError ? pProviderError : "" );
}

static BootstrapModuleLoadRecord_t *BootstrapModuleLoadFind( void *pModule )
{
	for ( BootstrapModuleLoadRecord_t *pRecord = g_pBootstrapModuleLoads;
		pRecord; pRecord = pRecord->m_pNext )
	{
		if ( pRecord->m_pModule == pModule )
			return pRecord;
	}
	return NULL;
}

static void BootstrapModuleLoadRemember(
	void *pModule,
	const char *pRequester,
	int nSourceLine,
	const char *pRequestedPath,
	const char *pResolvedPath,
	int nProviderResult,
	const char *pProviderError )
{
	const unsigned long long nLoadId = g_nNextBootstrapModuleLoadId++;
	if ( pModule )
	{
		BootstrapModuleLoadRecord_t *pRecord =
			(BootstrapModuleLoadRecord_t *)malloc( sizeof( *pRecord ) );
		if ( pRecord )
		{
			memset( pRecord, 0, sizeof( *pRecord ) );
			pRecord->m_pNext = g_pBootstrapModuleLoads;
			g_pBootstrapModuleLoads = pRecord;
			pRecord->m_pModule = pModule;
			pRecord->m_nLoadId = nLoadId;
			pRecord->m_nStartedAtMicroseconds = BootstrapModuleLoadTime();
			pRecord->m_pRequester = pRequester;
			pRecord->m_nSourceLine = nSourceLine;
			BootstrapModuleLoadCopy( pRecord->m_szRequestedPath,
				sizeof( pRecord->m_szRequestedPath ), pRequestedPath );
			BootstrapModuleLoadCopy( pRecord->m_szResolvedPath,
				sizeof( pRecord->m_szResolvedPath ), pResolvedPath );
		}
	}
	BootstrapModuleLoadEmit( 0, nLoadId, pRequester, nSourceLine,
		pRequestedPath, pResolvedPath, "", pModule != NULL, 0,
		nProviderResult, pProviderError );
}

static void BootstrapModuleLoadForget(
	void *pModule,
	const char *pRequester,
	int nSourceLine,
	bool bSuccess,
	int nProviderResult,
	const char *pProviderError )
{
	BootstrapModuleLoadRecord_t *pRecord = BootstrapModuleLoadFind( pModule );
	if ( pRecord )
	{
		BootstrapModuleLoadEmit( 3, pRecord->m_nLoadId,
			pRecord->m_pRequester, pRecord->m_nSourceLine,
			pRecord->m_szRequestedPath, pRecord->m_szResolvedPath, "",
			bSuccess,
			BootstrapModuleLoadTime() - pRecord->m_nStartedAtMicroseconds,
			nProviderResult, pProviderError );
		if ( bSuccess )
		{
			BootstrapModuleLoadRecord_t **ppRecord =
				&g_pBootstrapModuleLoads;
			while ( *ppRecord && *ppRecord != pRecord )
				ppRecord = &( *ppRecord )->m_pNext;
			if ( *ppRecord )
			{
				*ppRecord = pRecord->m_pNext;
				free( pRecord );
			}
		}
	}
	else
	{
		BootstrapModuleLoadEmit( 3, 0, pRequester, nSourceLine,
			"", "", "", bSuccess, 0, nProviderResult, pProviderError );
	}
}

static void BootstrapModuleLoadReportOutstanding()
{
	while ( g_pBootstrapModuleLoads )
	{
		BootstrapModuleLoadRecord_t *pRecord = g_pBootstrapModuleLoads;
		g_pBootstrapModuleLoads = pRecord->m_pNext;
		BootstrapModuleLoadEmit( 3, pRecord->m_nLoadId,
			pRecord->m_pRequester, pRecord->m_nSourceLine,
			pRecord->m_szRequestedPath, pRecord->m_szResolvedPath, "", false,
			BootstrapModuleLoadTime() - pRecord->m_nStartedAtMicroseconds,
			-1, "module still loaded at process shutdown" );
		free( pRecord );
	}
}

static void BootstrapModuleLoadTelemetry_Init(
	int argc, char **argv, const char *pCommandLine = NULL )
{
	for ( int i = 1; i < argc; ++i )
	{
		if ( argv[i] && !strcmp( argv[i], "-moduleloadtelemetry" ) )
			g_bBootstrapModuleLoadTelemetryEnabled = true;
	}
	if ( pCommandLine && strstr( pCommandLine, "-moduleloadtelemetry" ) )
		g_bBootstrapModuleLoadTelemetryEnabled = true;
	if ( g_bBootstrapModuleLoadTelemetryEnabled )
		atexit( BootstrapModuleLoadReportOutstanding );
}

#if defined( _WIN32 )

static void BootstrapWideToUtf8(
	const wchar_t *pSource, char *pDestination, int nDestinationSize )
{
	if ( !pSource || !WideCharToMultiByte( CP_UTF8, 0, pSource, -1,
		pDestination, nDestinationSize, NULL, NULL ) )
	{
		pDestination[0] = '\0';
	}
}

static void BootstrapWindowsErrorText(
	DWORD nError, char *pError, int nErrorSize )
{
	if ( nError == ERROR_SUCCESS )
	{
		pError[0] = '\0';
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

static HMODULE BootstrapLoadLibraryExA(
	LPCSTR pPath, HANDLE hFile, DWORD nFlags,
	const char *pRequester, int nSourceLine )
{
	SetLastError( ERROR_SUCCESS );
	HMODULE hModule = LoadLibraryExA( pPath, hFile, nFlags );
	const DWORD nError = hModule ? ERROR_SUCCESS : GetLastError();
	char szResolved[2048];
	char szError[512];
	BootstrapModuleLoadCopy( szResolved, sizeof( szResolved ), pPath );
	if ( hModule && !GetModuleFileNameA(
		hModule, szResolved, sizeof( szResolved ) ) )
		BootstrapModuleLoadCopy( szResolved, sizeof( szResolved ), pPath );
	BootstrapWindowsErrorText( nError, szError, sizeof( szError ) );
	BootstrapModuleLoadRemember( hModule, pRequester, nSourceLine,
		pPath, szResolved, (int)nError, szError );
	return hModule;
}

static HMODULE BootstrapLoadLibraryExW(
	LPCWSTR pPath, HANDLE hFile, DWORD nFlags,
	const char *pRequester, int nSourceLine )
{
	char szRequested[2048];
	BootstrapWideToUtf8( pPath, szRequested, sizeof( szRequested ) );
	SetLastError( ERROR_SUCCESS );
	HMODULE hModule = LoadLibraryExW( pPath, hFile, nFlags );
	const DWORD nError = hModule ? ERROR_SUCCESS : GetLastError();
	char szResolved[2048];
	char szError[512];
	BootstrapModuleLoadCopy( szResolved, sizeof( szResolved ), szRequested );
	if ( hModule && !GetModuleFileNameA(
		hModule, szResolved, sizeof( szResolved ) ) )
		BootstrapModuleLoadCopy( szResolved, sizeof( szResolved ), szRequested );
	BootstrapWindowsErrorText( nError, szError, sizeof( szError ) );
	BootstrapModuleLoadRemember( hModule, pRequester, nSourceLine,
		szRequested, szResolved, (int)nError, szError );
	return hModule;
}

static FARPROC BootstrapGetProcAddress(
	HMODULE hModule, LPCSTR pEntryPoint,
	const char *pRequester, int nSourceLine )
{
	SetLastError( ERROR_SUCCESS );
	FARPROC pResult = GetProcAddress( hModule, pEntryPoint );
	const DWORD nError = pResult ? ERROR_SUCCESS : GetLastError();
	char szError[512];
	BootstrapWindowsErrorText( nError, szError, sizeof( szError ) );
	BootstrapModuleLoadRecord_t *pRecord = BootstrapModuleLoadFind( hModule );
	BootstrapModuleLoadEmit( 1, pRecord ? pRecord->m_nLoadId : 0,
		pRecord ? pRecord->m_pRequester : pRequester,
		pRecord ? pRecord->m_nSourceLine : nSourceLine,
		pRecord ? pRecord->m_szRequestedPath : "",
		pRecord ? pRecord->m_szResolvedPath : "", pEntryPoint,
		pResult != NULL, 0, (int)nError, szError );
	return pResult;
}

static BOOL BootstrapFreeLibrary(
	HMODULE hModule, const char *pRequester, int nSourceLine )
{
	SetLastError( ERROR_SUCCESS );
	const BOOL bResult = FreeLibrary( hModule );
	const DWORD nError = bResult ? ERROR_SUCCESS : GetLastError();
	char szError[512];
	BootstrapWindowsErrorText( nError, szError, sizeof( szError ) );
	BootstrapModuleLoadForget( hModule, pRequester, nSourceLine,
		bResult != FALSE, (int)nError, szError );
	return bResult;
}

#undef LoadLibraryA
#undef LoadLibraryW
#undef LoadLibraryExA
#undef LoadLibraryExW
#undef GetProcAddress
#undef FreeLibrary
#define LoadLibraryA( path ) BootstrapLoadLibraryExA( \
	path, NULL, 0, __FILE__, __LINE__ )
#define LoadLibraryW( path ) BootstrapLoadLibraryExW( \
	path, NULL, 0, __FILE__, __LINE__ )
#define LoadLibraryExA( path, file, flags ) BootstrapLoadLibraryExA( \
	path, file, flags, __FILE__, __LINE__ )
#define LoadLibraryExW( path, file, flags ) BootstrapLoadLibraryExW( \
	path, file, flags, __FILE__, __LINE__ )
#define GetProcAddress( module, entry ) BootstrapGetProcAddress( \
	module, entry, __FILE__, __LINE__ )
#define FreeLibrary( module ) BootstrapFreeLibrary( \
	module, __FILE__, __LINE__ )

#elif defined( POSIX )

static void *BootstrapDlopen(
	const char *pPath, int nMode,
	const char *pRequester, int nSourceLine )
{
	dlerror();
	errno = 0;
	void *pModule = dlopen( pPath, nMode );
	const int nError = pModule ? 0 : ( errno ? errno : -1 );
	const char *pError = pModule ? "" : dlerror();
	char szResolved[PATH_MAX];
	BootstrapModuleLoadCopy( szResolved, sizeof( szResolved ), pPath );
	char szCanonical[PATH_MAX];
	if ( pPath && realpath( pPath, szCanonical ) )
		BootstrapModuleLoadCopy( szResolved, sizeof( szResolved ), szCanonical );
#if defined( __GLIBC__ )
	link_map *pMap = NULL;
	if ( pModule && dlinfo( pModule, RTLD_DI_LINKMAP, &pMap ) == 0 &&
		pMap && pMap->l_name && pMap->l_name[0] )
	{
		if ( realpath( pMap->l_name, szCanonical ) )
			BootstrapModuleLoadCopy(
				szResolved, sizeof( szResolved ), szCanonical );
		else
			BootstrapModuleLoadCopy(
				szResolved, sizeof( szResolved ), pMap->l_name );
	}
#endif
	BootstrapModuleLoadRemember( pModule, pRequester, nSourceLine,
		pPath, szResolved, nError, pError );
	return pModule;
}

static void *BootstrapDlsym(
	void *pModule, const char *pEntryPoint,
	const char *pRequester, int nSourceLine )
{
	dlerror();
	errno = 0;
	void *pResult = dlsym( pModule, pEntryPoint );
	const char *pError = dlerror();
	const bool bSuccess = pError == NULL;
	BootstrapModuleLoadRecord_t *pRecord = BootstrapModuleLoadFind( pModule );
	BootstrapModuleLoadEmit( 1, pRecord ? pRecord->m_nLoadId : 0,
		pRecord ? pRecord->m_pRequester : pRequester,
		pRecord ? pRecord->m_nSourceLine : nSourceLine,
		pRecord ? pRecord->m_szRequestedPath : "",
		pRecord ? pRecord->m_szResolvedPath : "", pEntryPoint,
		bSuccess, 0, bSuccess ? 0 : ( errno ? errno : -1 ), pError );
	return pResult;
}

static int BootstrapDlclose(
	void *pModule, const char *pRequester, int nSourceLine )
{
	dlerror();
	errno = 0;
	const int nResult = dlclose( pModule );
	const char *pError = nResult == 0 ? "" : dlerror();
	BootstrapModuleLoadForget( pModule, pRequester, nSourceLine,
		nResult == 0, nResult == 0 ? 0 : ( errno ? errno : nResult ), pError );
	return nResult;
}

#undef dlopen
#undef dlsym
#undef dlclose
#define dlopen( path, mode ) BootstrapDlopen( \
	path, mode, __FILE__, __LINE__ )
#define dlsym( module, entry ) BootstrapDlsym( \
	module, entry, __FILE__, __LINE__ )
#define dlclose( module ) BootstrapDlclose( \
	module, __FILE__, __LINE__ )

#endif

#endif // TIER0_BOOTSTRAP_MODULE_LOAD_TELEMETRY_H
