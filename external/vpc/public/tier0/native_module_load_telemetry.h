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
	HMODULE m_hModule;
	unsigned __int64 m_nLoadId;
	unsigned __int64 m_nStartedAt;
	const char *m_pRequester;
	int m_nSourceLine;
	char m_szRequested[2048];
	char m_szResolved[2048];
};

static VpcModuleLoadRecord_t g_VpcModuleLoads[32];
static int g_nVpcModuleLoads;

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
	for ( int i = g_nVpcModuleLoads - 1; i >= 0; --i )
		if ( g_VpcModuleLoads[i].m_hModule == hModule )
			return &g_VpcModuleLoads[i];
	return NULL;
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
	if ( hModule && g_nVpcModuleLoads < 32 )
	{
		VpcModuleLoadRecord_t &record = g_VpcModuleLoads[g_nVpcModuleLoads++];
		record.m_hModule = hModule;
		record.m_nLoadId = nLoadId;
		record.m_nStartedAt = VpcModuleLoadTime();
		record.m_pRequester = pRequester;
		record.m_nSourceLine = nSourceLine;
		VpcModuleLoadCopy( record.m_szRequested, sizeof( record.m_szRequested ), pPath );
		VpcModuleLoadCopy( record.m_szResolved, sizeof( record.m_szResolved ), szResolved );
	}
	VpcModuleLoadEmit( 0, nLoadId, pRequester, nSourceLine,
		pPath, szResolved, "", hModule != NULL, 0, (int)nError, szError );
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
		const int nIndex = (int)( pRecord - g_VpcModuleLoads );
		for ( int i = nIndex + 1; i < g_nVpcModuleLoads; ++i )
			g_VpcModuleLoads[i - 1] = g_VpcModuleLoads[i];
		--g_nVpcModuleLoads;
	}
	return bResult;
}

class CVpcModuleLoadShutdownReporter
{
public:
	~CVpcModuleLoadShutdownReporter()
	{
		while ( g_nVpcModuleLoads > 0 )
		{
			VpcModuleLoadRecord_t &record =
				g_VpcModuleLoads[--g_nVpcModuleLoads];
			VpcModuleLoadEmit( 3, record.m_nLoadId,
				record.m_pRequester, record.m_nSourceLine,
				record.m_szRequested, record.m_szResolved, "", false,
				VpcModuleLoadTime() - record.m_nStartedAt,
				-1, "module still loaded at process shutdown" );
		}
	}
};

static CVpcModuleLoadShutdownReporter g_VpcModuleLoadShutdownReporter;

#undef LoadLibraryA
#undef LoadLibraryExA
#undef GetProcAddress
#undef FreeLibrary
#define LoadLibraryA( path ) VpcLoadLibraryExA( \
	path, NULL, 0, __FILE__, __LINE__ )
#define LoadLibraryExA( path, file, flags ) VpcLoadLibraryExA( \
	path, file, flags, __FILE__, __LINE__ )
#define GetProcAddress( module, entry ) VpcGetProcAddress( \
	module, entry, __FILE__, __LINE__ )
#define FreeLibrary( module ) VpcFreeLibrary( \
	module, __FILE__, __LINE__ )

#endif // _WIN32

#endif // VPC_TIER0_NATIVE_MODULE_LOAD_TELEMETRY_H
