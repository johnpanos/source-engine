//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "tier0/platform.h"
#include "tier0/vcrmode.h"
#include "tier0/memalloc.h"
#include "tier0/dbg.h"
#include "tier0/threadtools.h"
#include "foundation_facade.h"
#include <algorithm>
#include <vector>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#if defined(OSX) || defined(PLATFORM_BSD)
# ifdef PLATFORM_BSD
#  include <sys/proc.h>
#  include <sys/user.h>
# else
#  include <mach/mach.h>
#  include <mach/mach_time.h>
# endif
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#ifdef LINUX
#include <time.h>
#include <fcntl.h>
#endif
#ifdef ANDROID
#include <linux/stat.h>
#endif

#include "tier0/memdbgon.h"
// Benchmark mode uses this heavy-handed method 
extern VCRMode_t g_VCRMode;

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

size_t ApproximateProcessMemoryUsage( void )
{
/*
From http://man7.org/linux/man-pages/man5/proc.5.html:
       /proc/[pid]/statm
              Provides information about memory usage, measured in pages.
              The columns are:

                  size       (1) total program size
                             (same as VmSize in /proc/[pid]/status)
                  resident   (2) resident set size
                             (same as VmRSS in /proc/[pid]/status)
                  share      (3) shared pages (i.e., backed by a file)
                  text       (4) text (code)
                  lib        (5) library (unused in Linux 2.6)
                  data       (6) data + stack
                  dt         (7) dirty pages (unused in Linux 2.6)
*/

// This returns the resident memory size (RES column in 'top') in bytes.
	size_t nRet = 0;
	FILE *pFile = fopen( "/proc/self/statm", "r" );
	if ( pFile )
	{
		size_t nSize, nResident, nShare, nText, nLib_Unused, nDataPlusStack, nDt_Unused;
		if ( fscanf( pFile, "%zu %zu %zu %zu %zu %zu %zu", &nSize, &nResident, &nShare, &nText, &nLib_Unused, &nDataPlusStack, &nDt_Unused ) >= 2 )
		{
			nRet = 4096 * nResident;
		}
		fclose( pFile );
	}
	return nRet;
}

// Tier 0's time base is the facade's monotonic clock (R103 T1). The legacy
// version added the current nanoseconds without subtracting the start's, so it
// began anywhere in [0, 1) s; this one begins at 0 on the first call.
double Plat_FloatTime()
{
	if ( g_bBenchmarkMode )
	{
		g_FakeBenchmarkTime += g_FakeBenchmarkTimeInc;
		return g_FakeBenchmarkTime;
	}
	return tier0_facade::SecondsSinceStart();
}

unsigned int Plat_MSTime()
{
	if ( g_bBenchmarkMode )
	{
		g_FakeBenchmarkTime += g_FakeBenchmarkTimeInc;
		return (unsigned int)(g_FakeBenchmarkTime * 1000.0);
	}
	return ( uint )( Plat_FloatTime() * 1000 );
}

void Plat_ThreadSleep( unsigned nMilliseconds )
{
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

uint64 Plat_USTime()
{
	if ( g_bBenchmarkMode )
	{
		g_FakeBenchmarkTime += g_FakeBenchmarkTimeInc;
		return (unsigned int)(g_FakeBenchmarkTime * 1000000.0);
	}
	return ( uint64 )( Plat_FloatTime() * 1000000 );
}

// Wraps the thread-safe versions of ctime. buf must be at least 26 bytes 
char *Plat_ctime( const time_t *timep, char *buf, size_t bufsize )
{
	return ctime_r( timep, buf );
}

// Wraps the thread-safe versions of gmtime
struct tm *Plat_gmtime( const time_t *timep, struct tm *result )
{
	return gmtime_r( timep, result );
}

time_t Plat_timegm( struct tm *timeptr )
{
	return timegm( timeptr );
}

// Wraps the thread-safe versions of localtime
struct tm *Plat_localtime( const time_t *timep, struct tm *result )
{
	return localtime_r( timep, result );
}

bool vtune( bool resume )
{
  return 0;
}


// -------------------------------------------------------------------------------------------------- //
// Memory stuff.
// -------------------------------------------------------------------------------------------------- //


// The live debugger state from Tier 0's process environment (R103): TracerPid
// on Linux and Android, P_TRACED on Apple and FreeBSD.
bool Plat_IsInDebugSession()
{
	return tier0_facade::ProcessEnvironment().GetDebuggerState() == platform::DebuggerState::kAttached;
}

void Plat_DebugString( const char * psz )
{
	printf( "%s", psz );
}

static char g_CmdLine[ 2048 ];
PLATFORM_INTERFACE void Plat_SetCommandLine( const char *cmdLine )
{
	strncpy( g_CmdLine, cmdLine, sizeof(g_CmdLine) );
	g_CmdLine[ sizeof(g_CmdLine) -1 ] = 0;
}

PLATFORM_INTERFACE const tchar *Plat_GetCommandLine()
{
#ifdef LINUX
	// The arguments as the OS reports them (Tier 0's process environment,
	// R103), each followed by a space: the legacy read of /proc/self/cmdline
	// turned every NUL, the last included, into one.
	if( !g_CmdLine[ 0 ] )
	{
		const platform::IProcessEnvironment &environment = tier0_facade::ProcessEnvironment();
		size_t nLength = 0;
		for ( int i = 0; i < environment.ArgumentCount(); ++i )
		{
			char szArgument[ sizeof( g_CmdLine ) ];
			if ( environment.GetArgument( i, szArgument, sizeof( szArgument ) ) < 0 )
				szArgument[ 0 ] = 0;
			const size_t nArgument = strlen( szArgument );
			if ( nLength + nArgument + 1 >= sizeof( g_CmdLine ) )
				break;
			memcpy( g_CmdLine + nLength, szArgument, nArgument );
			nLength += nArgument;
			g_CmdLine[ nLength++ ] = ' ';
		}
		g_CmdLine[ nLength ] = '\0';
		Assert( g_CmdLine[ 0 ] );
	}
#endif // LINUX

	return g_CmdLine;
}

PLATFORM_INTERFACE const char *Plat_GetCommandLineA()
{
	return Plat_GetCommandLine();
}

PLATFORM_INTERFACE bool GetMemoryInformation( MemoryInformation *pOutMemoryInfo )
{
	#if defined( LINUX ) || defined( OSX ) || defined(PLATFORM_BSD)
		return false;
	#else
		#error "Need to fill out GetMemoryInformation or at least return false for this platform"
	#endif
}


PLATFORM_INTERFACE bool Is64BitOS()
{
#if defined OSX
	return true;
#elif defined(LINUX) || defined(PLATFORM_BSD)
	FILE *pp = popen( "uname -m", "r" );
	if ( pp != NULL )
	{
		char rgchArchString[256];
		fgets( rgchArchString, sizeof( rgchArchString ), pp );
		pclose( pp );
		if ( !strncasecmp( rgchArchString, "x86_64", strlen( "x86_64" ) ) )
			return true;
	}
#else
	Assert( !"implement Is64BitOS" );
#endif
	return false;
}

PLATFORM_INTERFACE void Plat_ExitProcess( int nCode )
{
	_exit( nCode );
}


static int s_nWatchDogTimerTimeScale = 0;
static bool s_bInittedWD = false;
static int s_WatchdogTime = 0;
static Plat_WatchDogHandlerFunction_t s_pWatchDogHandlerFunction;

static void InitWatchDogTimer( void )
{
	if( !strstr( g_CmdLine, "-nowatchdog" ) )
	{
#ifdef _DEBUG
		s_nWatchDogTimerTimeScale = 10;						// debug is slow
#else
		s_nWatchDogTimerTimeScale = 1;
#endif

	}

}

// SIGALRM handler. Used by Watchdog timer code.
static void WatchDogHandler( int s )
{
	Plat_DebugString( "WatchDog! Server took too long to process (probably infinite loop).\n" );

	DebuggerBreakIfDebugging();

	if ( s_pWatchDogHandlerFunction )
	{
		s_pWatchDogHandlerFunction();
	}
	else
	{
		// force a crash
		abort();
	}
}

// The watchdog provider's fire callback: still the signal handler's context.
static void WatchDogFire( void * )
{
	WatchDogHandler( SIGALRM );
}

// watchdog timer support
PLATFORM_INTERFACE void Plat_BeginWatchdogTimer( int nSecs )
{
	if ( !s_bInittedWD )
	{
		s_bInittedWD = true;
		InitWatchDogTimer();
	}

	nSecs *= s_nWatchDogTimerTimeScale;
	nSecs = MIN( nSecs, 5 * 60 );							// no more than 5 minutes no matter what
	if ( nSecs )
	{
		s_WatchdogTime = nSecs;
		// Tier 0's watchdog (R103): SIGALRM through the diagnostics provider.
		tier0_facade::Watchdog().Arm( (unsigned)nSecs, WatchDogFire, NULL );
	}
}

PLATFORM_INTERFACE void Plat_EndWatchdogTimer( void )
{
	tier0_facade::Watchdog().Disarm();
	s_WatchdogTime = 0;
}

PLATFORM_INTERFACE int Plat_GetWatchdogTime( void )
{
	return s_WatchdogTime;
}

PLATFORM_INTERFACE void Plat_SetWatchdogHandlerFunction( Plat_WatchDogHandlerFunction_t function )
{
	s_pWatchDogHandlerFunction = function;
}


// Turn off memdbg macros (turned on up top) since this is included like a header
#include "tier0/memdbgoff.h"


