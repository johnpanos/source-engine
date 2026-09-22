// Tool-local native loader telemetry for the standalone VPC composition.
#ifndef VPC_TIER0_NATIVE_MODULE_LOAD_TELEMETRY_H
#define VPC_TIER0_NATIVE_MODULE_LOAD_TELEMETRY_H

#if defined( _WIN32 )

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct VpcModuleLoadRecord_t
{
	VpcModuleLoadRecord_t *m_pNext;
	HMODULE m_hModule;
	unsigned __int64 m_nLoadId;
	unsigned __int64 m_nStartedAt;
	const char *m_pRequester;
	int m_nSourceLine;
	char m_szRequested[2048];
	char m_szResolved[2048];
};

inline VpcModuleLoadRecord_t *&VpcModuleLoadRecords()
{
	static VpcModuleLoadRecord_t *s_pRecords;
	return s_pRecords;
}

static unsigned __int64 VpcModuleLoadTime()
{
	LARGE_INTEGER value, frequency;
	QueryPerformanceCounter( &value );
	QueryPerformanceFrequency( &frequency );
	return value.QuadPart * 1000000ULL / frequency.QuadPart;
}

static bool VpcModuleLoadTelemetryEnabled()
{
	const char *pEnvironment = getenv( "SOURCE_MODULE_LOAD_TELEMETRY" );
	return ( pEnvironment && pEnvironment[0] && pEnvironment[0] != '0' ) ||
		strstr( GetCommandLineA(), "-moduleloadtelemetry" ) != NULL;
}

static void VpcModuleLoadCopy(
	char *pDestination, int nSize, const char *pSource )
{
	strncpy( pDestination, pSource ? pSource : "", nSize );
	pDestination[nSize - 1] = '\0';
}

static void VpcModuleLoadWideCopy(
	char *pDestination, int nSize, const wchar_t *pSource )
{
	if ( !pSource || !WideCharToMultiByte( CP_UTF8, 0, pSource, -1,
		pDestination, nSize, NULL, NULL ) )
	{
		VpcModuleLoadCopy( pDestination, nSize, "" );
	}
}

static void VpcModuleLoadEmit(
	int nOperation, unsigned __int64 nLoadId,
	const char *pRequester, int nSourceLine,
	const char *pRequested, const char *pResolved, const char *pEntry,
	bool bSuccess, unsigned __int64 nLifetime,
	int nProviderResult, const char *pError )
{
	if ( !VpcModuleLoadTelemetryEnabled() )
		return;
	char message[8192];
	_snprintf( message, sizeof( message ) - 1,
		"ModuleLoadTelemetry: op=%d id=%I64u requester=%s:%d requested=%s resolved=%s entry=%s success=%d lifetime_us=%I64u provider_result=%d error=%s\n",
		nOperation, nLoadId, pRequester ? pRequester : "", nSourceLine,
		pRequested ? pRequested : "", pResolved ? pResolved : "",
		pEntry ? pEntry : "", bSuccess ? 1 : 0, nLifetime,
		nProviderResult, pError ? pError : "" );
	message[sizeof( message ) - 1] = '\0';
	fputs( message, stderr );
	OutputDebugStringA( message );
}

static VpcModuleLoadRecord_t *VpcModuleLoadFind( HMODULE hModule )
{
	for ( VpcModuleLoadRecord_t *pRecord = VpcModuleLoadRecords();
		pRecord; pRecord = pRecord->m_pNext )
	{
		if ( pRecord->m_hModule == hModule )
			return pRecord;
	}
	return NULL;
}

static void VpcModuleLoadRemember(
	HMODULE hModule, unsigned __int64 nLoadId,
	const char *pRequester, int nSourceLine,
	const char *pRequested, const char *pResolved )
{
	if ( !hModule )
		return;
	VpcModuleLoadRecord_t *pRecord =
		(VpcModuleLoadRecord_t *)malloc( sizeof( *pRecord ) );
	if ( !pRecord )
		return;
	memset( pRecord, 0, sizeof( *pRecord ) );
	pRecord->m_pNext = VpcModuleLoadRecords();
	VpcModuleLoadRecords() = pRecord;
	pRecord->m_hModule = hModule;
	pRecord->m_nLoadId = nLoadId;
	pRecord->m_nStartedAt = VpcModuleLoadTime();
	pRecord->m_pRequester = pRequester;
	pRecord->m_nSourceLine = nSourceLine;
	VpcModuleLoadCopy( pRecord->m_szRequested,
		sizeof( pRecord->m_szRequested ), pRequested );
	VpcModuleLoadCopy( pRecord->m_szResolved,
		sizeof( pRecord->m_szResolved ), pResolved );
}

static HMODULE VpcLoadLibraryExA(
	LPCSTR pPath, HANDLE hFile, DWORD nFlags,
	const char *pRequester, int nSourceLine )
{
	SetLastError( ERROR_SUCCESS );
	HMODULE hModule = LoadLibraryExA( pPath, hFile, nFlags );
	const DWORD nError = hModule ? ERROR_SUCCESS : GetLastError();
	char szResolved[2048];
	VpcModuleLoadCopy( szResolved, sizeof( szResolved ), pPath );
	if ( hModule && !GetModuleFileNameA( hModule, szResolved, sizeof( szResolved ) ) )
		VpcModuleLoadCopy( szResolved, sizeof( szResolved ), pPath );
	char szError[512] = "";
	if ( nError )
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, nError, 0, szError, sizeof( szError ), NULL );
	const unsigned __int64 nLoadId =
		( VpcModuleLoadTime() << 8 ) ^ (unsigned __int64)(UINT_PTR)hModule ^
		(unsigned int)nSourceLine;
	VpcModuleLoadRemember( hModule, nLoadId, pRequester, nSourceLine,
		pPath, szResolved );
	VpcModuleLoadEmit( 0, nLoadId, pRequester, nSourceLine,
		pPath, szResolved, "", hModule != NULL, 0, (int)nError, szError );
	return hModule;
}

static HMODULE VpcLoadLibraryExW(
	LPCWSTR pPath, HANDLE hFile, DWORD nFlags,
	const char *pRequester, int nSourceLine )
{
	SetLastError( ERROR_SUCCESS );
	HMODULE hModule = LoadLibraryExW( pPath, hFile, nFlags );
	const DWORD nError = hModule ? ERROR_SUCCESS : GetLastError();
	char szRequested[2048];
	VpcModuleLoadWideCopy( szRequested, sizeof( szRequested ), pPath );
	wchar_t wszResolved[2048];
	char szResolved[2048];
	VpcModuleLoadCopy( szResolved, sizeof( szResolved ), szRequested );
	if ( hModule && GetModuleFileNameW(
		hModule, wszResolved, sizeof( wszResolved ) / sizeof( wszResolved[0] ) ) )
	{
		VpcModuleLoadWideCopy( szResolved, sizeof( szResolved ), wszResolved );
	}
	char szError[512] = "";
	if ( nError )
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, nError, 0, szError, sizeof( szError ), NULL );
	const unsigned __int64 nLoadId =
		( VpcModuleLoadTime() << 8 ) ^ (unsigned __int64)(UINT_PTR)hModule ^
		(unsigned int)nSourceLine;
	VpcModuleLoadRemember( hModule, nLoadId, pRequester, nSourceLine,
		szRequested, szResolved );
	VpcModuleLoadEmit( 0, nLoadId, pRequester, nSourceLine,
		szRequested, szResolved, "", hModule != NULL, 0,
		(int)nError, szError );
	return hModule;
}

static FARPROC VpcGetProcAddress(
	HMODULE hModule, LPCSTR pEntry,
	const char *pRequester, int nSourceLine )
{
	SetLastError( ERROR_SUCCESS );
	FARPROC pResult = GetProcAddress( hModule, pEntry );
	const DWORD nError = pResult ? ERROR_SUCCESS : GetLastError();
	VpcModuleLoadRecord_t *pRecord = VpcModuleLoadFind( hModule );
	char szError[512] = "";
	if ( nError )
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, nError, 0, szError, sizeof( szError ), NULL );
	VpcModuleLoadEmit( 1, pRecord ? pRecord->m_nLoadId : 0,
		pRecord ? pRecord->m_pRequester : pRequester,
		pRecord ? pRecord->m_nSourceLine : nSourceLine,
		pRecord ? pRecord->m_szRequested : "",
		pRecord ? pRecord->m_szResolved : "", pEntry,
		pResult != NULL, 0, (int)nError, szError );
	return pResult;
}

static BOOL VpcFreeLibrary(
	HMODULE hModule, const char *pRequester, int nSourceLine )
{
	VpcModuleLoadRecord_t *pRecord = VpcModuleLoadFind( hModule );
	SetLastError( ERROR_SUCCESS );
	const BOOL bResult = FreeLibrary( hModule );
	const DWORD nError = bResult ? ERROR_SUCCESS : GetLastError();
	char szError[512] = "";
	if ( nError )
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, nError, 0, szError, sizeof( szError ), NULL );
	VpcModuleLoadEmit( 3, pRecord ? pRecord->m_nLoadId : 0,
		pRecord ? pRecord->m_pRequester : pRequester,
		pRecord ? pRecord->m_nSourceLine : nSourceLine,
		pRecord ? pRecord->m_szRequested : "",
		pRecord ? pRecord->m_szResolved : "", "", bResult != FALSE,
		pRecord ? VpcModuleLoadTime() - pRecord->m_nStartedAt : 0,
		(int)nError, szError );
	if ( bResult && pRecord )
	{
		VpcModuleLoadRecord_t **ppRecord = &VpcModuleLoadRecords();
		while ( *ppRecord && *ppRecord != pRecord )
			ppRecord = &( *ppRecord )->m_pNext;
		if ( *ppRecord )
		{
			*ppRecord = pRecord->m_pNext;
			free( pRecord );
		}
	}
	return bResult;
}

class CVpcModuleLoadShutdownReporter
{
public:
	~CVpcModuleLoadShutdownReporter()
	{
		while ( VpcModuleLoadRecords() )
		{
			VpcModuleLoadRecord_t *pRecord = VpcModuleLoadRecords();
			VpcModuleLoadRecords() = pRecord->m_pNext;
			VpcModuleLoadEmit( 3, pRecord->m_nLoadId,
				pRecord->m_pRequester, pRecord->m_nSourceLine,
				pRecord->m_szRequested, pRecord->m_szResolved, "", false,
				VpcModuleLoadTime() - pRecord->m_nStartedAt,
				-1, "module still loaded at process shutdown" );
			free( pRecord );
		}
	}
};

static CVpcModuleLoadShutdownReporter g_VpcModuleLoadShutdownReporter;

#undef LoadLibraryA
#undef LoadLibraryW
#undef LoadLibraryExA
#undef LoadLibraryExW
#undef GetProcAddress
#undef FreeLibrary
#define LoadLibraryA( path ) VpcLoadLibraryExA( \
	path, NULL, 0, __FILE__, __LINE__ )
#define LoadLibraryW( path ) VpcLoadLibraryExW( \
	path, NULL, 0, __FILE__, __LINE__ )
#define LoadLibraryExA( path, file, flags ) VpcLoadLibraryExA( \
	path, file, flags, __FILE__, __LINE__ )
#define LoadLibraryExW( path, file, flags ) VpcLoadLibraryExW( \
	path, file, flags, __FILE__, __LINE__ )
#define GetProcAddress( module, entry ) VpcGetProcAddress( \
	module, entry, __FILE__, __LINE__ )
#define FreeLibrary( module ) VpcFreeLibrary( \
	module, __FILE__, __LINE__ )

#endif // _WIN32

#endif // VPC_TIER0_NATIVE_MODULE_LOAD_TELEMETRY_H
