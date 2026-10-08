//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "pch_tier0.h"
#include <time.h>

#if defined( _WIN32 )
#include <errno.h>
#endif
#include <assert.h>
#include "tier0/platform.h"
#include "tier0/minidump.h"
#include "tier0/native_module_load_telemetry.h"
#include "tier0/vcrmode.h"
#include "tier0/threadtools.h"
#include "foundation_facade.h"
#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)
#include "tier0/memalloc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#endif

//our global error callback function. Note that this is not initialized, but static space guarantees this is NULL at app start.
//If you initialize, it will set to zero again when the CPP runs its static initializers, which could stomp the value if another
//CPP sets this value while initializing its static space
static ExitProcessWithErrorCBFn g_pfnExitProcessWithErrorCB; //= NULL

extern VCRMode_t g_VCRMode;

// Benchmark mode uses this heavy-handed method 
static bool g_bBenchmarkMode = false;
static double g_FakeBenchmarkTime = 0;
static double g_FakeBenchmarkTimeInc = 1.0 / 66.0;

bool Plat_IsInBenchmarkMode()
{
	return g_bBenchmarkMode;
}

void Plat_SetBenchmarkMode( bool bBenchmark )
{
	g_bBenchmarkMode = bBenchmark;
}

// Tier 0's time base is the facade's monotonic clock (R103 T1); as before, it
// starts at 0 on the first call.
double Plat_FloatTime()
{
	if ( g_bBenchmarkMode )
	{
		g_FakeBenchmarkTime += g_FakeBenchmarkTimeInc;
		return g_FakeBenchmarkTime;
	}
	return tier0_facade::SecondsSinceStart();
}

uint32 Plat_MSTime()
{
	if ( g_bBenchmarkMode )
	{
		g_FakeBenchmarkTime += g_FakeBenchmarkTimeInc;
		return (uint32)(g_FakeBenchmarkTime * 1000.0);
	}
	return (uint32)( Plat_FloatTime() * 1000.0 );
}

uint64 Plat_USTime()
{
	if ( g_bBenchmarkMode )
	{
		g_FakeBenchmarkTime += g_FakeBenchmarkTimeInc;
		return (uint64)(g_FakeBenchmarkTime * 1e6);
	}
	return (uint64)( Plat_FloatTime() * 1e6 );
}

void Plat_ThreadSleep( unsigned nMilliseconds )
{
#ifdef _WIN32_PC
	// As the inline ThreadSleep did: a 1 ms timer resolution, so short sleeps
	// do not round up to the default 15.6 ms tick.
	static bool bInitialized = false;
	if ( !bInitialized )
	{
		bInitialized = true;
		timeBeginPeriod( 1 );
	}
#endif
	tier0_facade::Threads().SleepFor( (uint64)nMilliseconds * 1000000ull );
}

void Plat_ThreadSleepMicroseconds( unsigned nMicroseconds )
{
	tier0_facade::Threads().SleepFor( (uint64)nMicroseconds * 1000ull );
}

void Plat_ThreadYield()
{
	tier0_facade::Threads().SleepFor( 0 );
}

uint64 Plat_MonotonicNanoseconds()
{
	return tier0_facade::MonotonicNanoseconds();
}

void GetCurrentDate( int *pDay, int *pMonth, int *pYear )
{
	struct tm *pNewTime;
	time_t long_time;

	time( &long_time );                /* Get time as long integer. */
	pNewTime = localtime( &long_time ); /* Convert to local time. */

	*pDay = pNewTime->tm_mday;
	*pMonth = pNewTime->tm_mon + 1;
	*pYear = pNewTime->tm_year + 1900;
}


// Wraps the thread-safe versions of ctime. buf must be at least 26 bytes 
char *Plat_ctime( const time_t *timep, char *buf, size_t bufsize )
{
	if ( EINVAL == ctime_s( buf, bufsize, timep ) )
		return NULL;
	else
		return buf;
}

bool Plat_GetModuleFileNameOf( const void *pAddress, char *pOut, int nMaxBytes )
{
	if ( !pOut || nMaxBytes <= 0 )
		return false;
	pOut[0] = 0;
#ifdef PLATFORM_WINDOWS_PC
	void *pModule = platform::Win32ModuleOfAddress( pAddress );
	return pModule && platform::Win32ModuleFileNameA( pModule, pOut, nMaxBytes ) != 0;
#else
	(void)pAddress;
	return false;
#endif
}

void Plat_GetModuleFilename( char *pOut, int nMaxBytes )
{
#ifdef PLATFORM_WINDOWS_PC
	if ( nMaxBytes <= 0 ||
	     platform::Win32ModuleFileNameA( nullptr, pOut, nMaxBytes ) >= (size_t)nMaxBytes )
		Error( "Plat_GetModuleFilename: The buffer given is too small (%d bytes).", nMaxBytes );
#else
	// We shouldn't need this on POSIX.
	Assert( false );
	pOut[0] = 0x00;    // Null the returned string in release builds
#endif
}

void Plat_ExitProcess( int nCode )
{
#if defined( _WIN32 )
	// We don't want global destructors in our process OR in any DLL to get executed.
	// _exit() avoids calling global destructors in our module, but not in other DLLs.
	const char *pchCmdLineA = Plat_GetCommandLineA();
	if ( nCode || ( strstr( pchCmdLineA, "gc.exe" ) && strstr( pchCmdLineA, "gc.dll" ) && strstr( pchCmdLineA, "-gc" ) ) )
	{
		int *x = NULL; *x = 1; // cause a hard crash, GC is not allowed to exit voluntarily from gc.dll
	}
	TerminateProcess( GetCurrentProcess(), nCode );
#else
	_exit( nCode );
#endif
}

void Plat_ExitProcessWithError( int nCode, bool bGenerateMinidump )
{
	//try to delegate out if they have registered a callback
	if( g_pfnExitProcessWithErrorCB )
	{
		if( g_pfnExitProcessWithErrorCB( nCode ) )
			return;
	}

	//handle default behavior
	if( bGenerateMinidump )
	{
		//don't generate mini dumps in the debugger
		if( !Plat_IsInDebugSession() )
		{
			WriteMiniDump();
		}
	}

	//and exit our process
	Plat_ExitProcess( nCode );
}

void Plat_SetExitProcessWithErrorCB( ExitProcessWithErrorCBFn pfnCB )
{
	g_pfnExitProcessWithErrorCB = pfnCB;	
}

// Wraps the thread-safe versions of gmtime
struct tm *Plat_gmtime( const time_t *timep, struct tm *result )
{
	if ( EINVAL == gmtime_s( result, timep ) )
		return NULL;
	else
		return result;
}


time_t Plat_timegm( struct tm *timeptr )
{
	return _mkgmtime( timeptr );
}


// Wraps the thread-safe versions of localtime
struct tm *Plat_localtime( const time_t *timep, struct tm *result )
{
	if ( EINVAL == localtime_s( result, timep ) )
		return NULL;
	else
		return result;
}


bool vtune( bool resume )
{
	static bool bInitialized = false;
	static void (__cdecl *VTResume)(void) = NULL;
	static void (__cdecl *VTPause) (void) = NULL;

	// Grab the Pause and Resume function pointers from the VTune DLL the first time through:
	if( !bInitialized )
	{
		bInitialized = true;

		HINSTANCE pVTuneDLL = LoadLibrary( "vtuneapi.dll" );

		if( pVTuneDLL )
		{
			VTResume = (void(__cdecl *)())GetProcAddress( pVTuneDLL, "VTResume" );
			VTPause  = (void(__cdecl *)())GetProcAddress( pVTuneDLL, "VTPause" );
		}
	}

	// Call the appropriate function, as indicated by the argument:
	if( resume && VTResume )
	{
		VTResume();
		return true;

	} 
	else if( !resume && VTPause )
	{
		VTPause();
		return true;
	}
	return false;
}

bool Plat_IsInDebugSession()
{
#if defined( _WIN32 )
	return tier0_facade::ProcessEnvironment().GetDebuggerState() == platform::DebuggerState::kAttached;
#elif defined( LINUX )
	#error This code is implemented in platform_posix.cpp
#else
	return false;
#endif
}

void Plat_DebugString( const char * psz )
{
#if defined( _WIN32 )
	if ( platform::IDebugOutput *output = tier0_facade::DebugOutput() )
		output->Write( platform::DiagnosticSeverity::kInfo, psz ); // the debugger channel (R103)
#endif
}


const tchar *Plat_GetCommandLine()
{
#ifdef TCHAR_IS_WCHAR
	return tier0_facade::ProcessEnvironment().RawCommandLineW();
#else
	return tier0_facade::ProcessEnvironment().RawCommandLineA();
#endif
}

bool GetMemoryInformation( MemoryInformation *pOutMemoryInfo )
{
	if ( !pOutMemoryInfo ) 
		return false;

	MEMORYSTATUSEX	memStat;
	ZeroMemory( &memStat, sizeof( MEMORYSTATUSEX ) );
	memStat.dwLength = sizeof( MEMORYSTATUSEX );

	if ( !GlobalMemoryStatusEx( &memStat ) ) 
		return false;

	const uint cOneMb = 1024 * 1024;

	switch ( pOutMemoryInfo->m_nStructVersion )
	{
	case 0:
		( *pOutMemoryInfo ).m_nPhysicalRamMbTotal     = memStat.ullTotalPhys / cOneMb;
		( *pOutMemoryInfo ).m_nPhysicalRamMbAvailable = memStat.ullAvailPhys / cOneMb;

		( *pOutMemoryInfo ).m_nVirtualRamMbTotal      = memStat.ullTotalVirtual / cOneMb;
		( *pOutMemoryInfo ).m_nVirtualRamMbAvailable  = memStat.ullAvailVirtual / cOneMb;
		break;

	default:
		return false;
	};

	return true;
}


const char *Plat_GetCommandLineA()
{
	return tier0_facade::ProcessEnvironment().RawCommandLineA();
}

//--------------------------------------------------------------------------------------------------
// Watchdog timer
//--------------------------------------------------------------------------------------------------
void Plat_BeginWatchdogTimer( int nSecs )
{
}
void Plat_EndWatchdogTimer( void )
{
}
int Plat_GetWatchdogTime( void )
{
	return 0;
}
void Plat_SetWatchdogHandlerFunction( Plat_WatchDogHandlerFunction_t function )
{
}

bool Is64BitOS()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	static LPFN_ISWOW64PROCESS pfnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle("kernel32"), "IsWow64Process" );

	static BOOL bIs64bit = FALSE;
	static bool bInitialized = false;
	if ( bInitialized ) 
		return bIs64bit == (BOOL)TRUE;
	else
	{
		bInitialized = true;
		return pfnIsWow64Process && pfnIsWow64Process(GetCurrentProcess(), &bIs64bit) && bIs64bit;
	}
}


// -------------------------------------------------------------------------------------------------- //
// Memory stuff. 
//
// DEPRECATED. Still here to support binary back compatability of tier0.dll
//
// -------------------------------------------------------------------------------------------------- //
#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)

typedef void (*Plat_AllocErrorFn)( unsigned long size );

void Plat_DefaultAllocErrorFn( unsigned long size )
{
}

Plat_AllocErrorFn g_AllocError = Plat_DefaultAllocErrorFn;
#endif

CRITICAL_SECTION g_AllocCS;
class CAllocCSInit
{
public:
	CAllocCSInit()
	{
		InitializeCriticalSection( &g_AllocCS );
	}
} g_AllocCSInit;

PLATFORM_INTERFACE void* Plat_Alloc( unsigned long size )
{
	EnterCriticalSection( &g_AllocCS );
#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)
		void *pRet = g_pMemAlloc->Alloc( size );
#else
		void *pRet = malloc( size );
#endif
	LeaveCriticalSection( &g_AllocCS );
	if ( pRet )
	{
		return pRet;
	}
	else
	{
#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)
		g_AllocError( size );
#endif
		return 0;
	}
}

PLATFORM_INTERFACE void* Plat_Realloc( void *ptr, unsigned long size )
{
	EnterCriticalSection( &g_AllocCS );
#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)
		void *pRet = g_pMemAlloc->Realloc( ptr, size );
#else
		void *pRet = realloc( ptr, size );
#endif
	LeaveCriticalSection( &g_AllocCS );
	if ( pRet )
	{
		return pRet;
	}
	else
	{
#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)
		g_AllocError( size );
#endif
		return 0;
	}
}

PLATFORM_INTERFACE void Plat_Free( void *ptr )
{
	EnterCriticalSection( &g_AllocCS );
#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)
		g_pMemAlloc->Free( ptr );
#else
		free( ptr );
#endif
	LeaveCriticalSection( &g_AllocCS );
}

#if !defined(STEAM) && !defined(NO_MALLOC_OVERRIDE)
PLATFORM_INTERFACE void Plat_SetAllocErrorFn( Plat_AllocErrorFn fn )
{
	g_AllocError = fn;
}
#endif

