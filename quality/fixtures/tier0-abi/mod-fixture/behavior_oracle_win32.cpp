// R103 behavior oracles for Windows PE, through Tier 0's exports only (no Tier 0
// headers), against the legacy Win32 results:
//   T1: Plat_FloatTime and Plat_MonotonicNanoseconds advance with real time.
//   T2: ThreadGetCurrentId is GetCurrentThreadId; ThreadSleep's export sleeps.
//   T3: Plat_GetCommandLineA is GetCommandLineA's text, the same buffer on
//       every call; no debugger is reported.
//   T5: the watchdog stays a no-op (it never fires); GetCallStack returns
//       real frames.
//   T6: Plat_GetModuleFilename is the executable's path, and
//       Plat_GetModuleFileNameOf finds the module of an address.
//
//   behavior_oracle_win32.exe      (linked against tier0.dll, beside it)
#include <windows.h>

#include <stdio.h>
#include <string.h>

// One import links tier0.dll; every other export is found by name.
extern "C" __declspec( dllimport ) double Plat_FloatTime();

namespace
{
int g_checks = 0;
int g_failures = 0;

void Check( bool ok, const char *what )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		printf( "FAIL %s\n", what );
	}
}

template <typename T> T Find( HMODULE module, const char *name )
{
	return reinterpret_cast<T>( reinterpret_cast<void *>( GetProcAddress( module, name ) ) );
}

volatile LONG g_fired = 0;
} // namespace

int main( int argc, char **argv )
{
	(void)argc;
	(void)argv;
	HMODULE tier0 = GetModuleHandleA( "tier0.dll" );
	if ( tier0 == nullptr || Plat_FloatTime() < 0.0 )
	{
		printf( "FAIL tier0 not loaded\nCONFORMANCE 1 1\n" );
		return 1;
	}

	// T1: time.
	auto floatTime = Find<double ( * )()>( tier0, "Plat_FloatTime" );
	auto nanoseconds = Find<unsigned long long ( * )()>( tier0, "Plat_MonotonicNanoseconds" );
	Check( floatTime && nanoseconds, "time exports present" );
	if ( floatTime && nanoseconds )
	{
		const double t0 = floatTime();
		const unsigned long long n0 = nanoseconds();
		Sleep( 100 );
		const double dt = floatTime() - t0;
		const unsigned long long dn = nanoseconds() - n0;
		Check( dt >= 0.09 && dt < 2.0, "Plat_FloatTime advances with Sleep(100)" );
		Check( dn >= 90000000ull && dn < 2000000000ull, "Plat_MonotonicNanoseconds advances with Sleep(100)" );
	}

	// T2: threads.
	auto threadId = Find<unsigned long ( * )()>( tier0, "ThreadGetCurrentId" );
	// ThreadSleep is inline in the header and calls this export.
	auto threadSleep = Find<void ( * )( unsigned )>( tier0, "Plat_ThreadSleep" );
	Check( threadId && threadSleep, "thread exports present" );
	if ( threadId && threadSleep )
	{
		Check( threadId() == GetCurrentThreadId(), "ThreadGetCurrentId is GetCurrentThreadId" );
		const ULONGLONG start = GetTickCount64();
		threadSleep( 50 );
		Check( GetTickCount64() - start >= 40, "Plat_ThreadSleep(50) sleeps" );
	}

	// T3: command line and debugger state.
	auto getLine = Find<const char *( * )()>( tier0, "Plat_GetCommandLineA" );
	auto inDebug = Find<bool ( * )()>( tier0, "Plat_IsInDebugSession" );
	Check( getLine && inDebug, "process exports present" );
	if ( getLine )
	{
		const char *line = getLine();
		Check( line != nullptr && strcmp( line, GetCommandLineA() ) == 0,
		       "Plat_GetCommandLineA is GetCommandLineA" );
		Check( getLine() == line, "the same buffer on every call" );
	}
	if ( inDebug )
	{
		Check( !inDebug(), "Plat_IsInDebugSession is false without a debugger" );
	}

	// T5: the watchdog is unsupported on Windows, as before: begin and end are
	// accepted and the handler never runs.
	auto setHandler = Find<void ( * )( void ( * )() )>( tier0, "Plat_SetWatchdogHandlerFunction" );
	auto begin = Find<void ( * )( int )>( tier0, "Plat_BeginWatchdogTimer" );
	auto end = Find<void ( * )()>( tier0, "Plat_EndWatchdogTimer" );
	Check( setHandler && begin && end, "watchdog exports present" );
	if ( setHandler && begin && end )
	{
		setHandler( [] { InterlockedIncrement( &g_fired ); } );
		begin( 1 );
		Sleep( 1500 );
		end();
		Check( g_fired == 0, "the watchdog never fires on Windows" );
	}

	auto getStack = Find<int ( * )( void **, int, int )>( tier0, "GetCallStack" );
	Check( getStack != nullptr, "GetCallStack present" );
	if ( getStack )
	{
		void *frames[16] = {};
		const int n = getStack( frames, 16, 0 );
		Check( n >= 2 && n <= 16 && frames[0] != nullptr, "GetCallStack returns frames" );
		Check( getStack( frames, 0, 0 ) == 0, "GetCallStack with no room returns none" );
	}

	// T6: module paths.
	auto moduleName = Find<void ( * )( char *, int )>( tier0, "Plat_GetModuleFilename" );
	auto moduleOf = Find<bool ( * )( const void *, char *, int )>( tier0, "Plat_GetModuleFileNameOf" );
	Check( moduleName && moduleOf, "module path exports present" );
	char expected[MAX_PATH] = {};
	GetModuleFileNameA( nullptr, expected, MAX_PATH );
	if ( moduleName )
	{
		char name[MAX_PATH] = {};
		moduleName( name, MAX_PATH );
		Check( strcmp( name, expected ) == 0, "Plat_GetModuleFilename is the executable" );
	}
	if ( moduleOf )
	{
		static int inThisModule;
		char name[MAX_PATH] = {};
		Check( moduleOf( &inThisModule, name, MAX_PATH ) && strcmp( name, expected ) == 0,
		       "Plat_GetModuleFileNameOf finds this executable" );
		char tier0Name[MAX_PATH] = {};
		GetModuleFileNameA( tier0, tier0Name, MAX_PATH );
		Check( moduleOf( reinterpret_cast<const void *>( floatTime ), name, MAX_PATH ) &&
		           strcmp( name, tier0Name ) == 0,
		       "Plat_GetModuleFileNameOf finds Tier 0" );
		Check( !moduleOf( nullptr, name, MAX_PATH ) && name[0] == 0,
		       "an address in no module has no name" );
	}

	printf( "CONFORMANCE %d %d\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
