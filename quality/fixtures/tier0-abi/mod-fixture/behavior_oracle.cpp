// R103 behavior oracles, through Tier 0's exports only (no Tier 0 headers):
//   T3: with nobody calling Plat_SetCommandLine, the command line equals the
//       legacy read of /proc/self/cmdline (every NUL, the last included,
//       turned into a space); no debugger is reported.
//   T5: the watchdog fires its handler once after its delay and not when
//       ended first; GetCallStack returns real frames.
// The host re-executes itself with known arguments.
#include "platform/contracts/dynamic_library.h"
#include "platform/posix/dynamic_library_provider.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <csignal>
#include <ctime>
#include <unistd.h>

namespace
{
class CQuietObserver final : public platform::IDynamicLibraryObserver
{
public:
	void OnLoad( const char *, const char *, const platform::IDynamicLibrary *,
		const platform::DynamicLibraryError & ) override
	{
	}
	void OnFindSymbol( const platform::IDynamicLibrary *, const char *, const void *,
		const platform::DynamicLibraryError & ) override
	{
	}
	void OnUnload( const platform::IDynamicLibrary * ) override {}
};
} // namespace

int main( int argc, char **argv )
{
	const char *args[] = { "oracle-host", "-game", "portal", "", "+map x y", "caf\xc3\xa9", nullptr };
	if ( getenv( "R103_ORACLE_CHILD" ) == nullptr )
	{
		if ( argc != 2 )
		{
			printf( "usage: behavior_oracle <libtier0.so>\nCONFORMANCE 1 1\n" );
			return 1;
		}
		setenv( "R103_ORACLE_CHILD", argv[1], 1 );
		execv( "/proc/self/exe", const_cast<char *const *>( args ) );
		printf( "FAIL re-exec\nCONFORMANCE 1 1\n" );
		return 1;
	}
	CQuietObserver observer;
	auto loader = platform::CreatePosixDynamicLibraryLoader( observer );
	platform::IDynamicLibrary *tier0 = loader->Load( getenv( "R103_ORACLE_CHILD" ), nullptr );
	if ( tier0 == nullptr )
	{
		printf( "FAIL load tier0\nCONFORMANCE 1 1\n" );
		return 1;
	}
	typedef const char *( *GetLine )();
	typedef bool ( *InDebug )();
	auto getLine = reinterpret_cast<GetLine>( tier0->FindSymbol( "Plat_GetCommandLineA", nullptr ) );
	auto inDebug = reinterpret_cast<InDebug>( tier0->FindSymbol( "Plat_IsInDebugSession", nullptr ) );
	int checks = 0, failures = 0;
	auto check = [&]( bool ok, const char *what ) {
		++checks;
		if ( !ok )
		{
			++failures;
			printf( "FAIL %s\n", what );
		}
	};
	check( getLine != nullptr && inDebug != nullptr, "exports present" );
	if ( getLine != nullptr )
	{
		// The legacy result: the raw bytes of /proc/self/cmdline, NULs to spaces.
		std::string expected;
		for ( int i = 0; args[i] != nullptr; ++i )
		{
			expected += args[i];
			expected += ' ';
		}
		const char *line = getLine();
		check( line != nullptr && expected == line, "Plat_GetCommandLineA equals the legacy /proc read" );
		check( getLine() == line, "the same buffer on every call" );
		if ( line != nullptr && expected != line )
		{
			printf( "  expected [%s]\n  got      [%s]\n", expected.c_str(), line );
		}
	}
	if ( inDebug != nullptr )
	{
		check( !inDebug(), "Plat_IsInDebugSession is false without a debugger" );
	}

	// T5: the watchdog.
	typedef void ( *Handler )();
	typedef void ( *SetHandler )( Handler );
	typedef void ( *Begin )( int );
	typedef void ( *End )();
	auto setHandler = reinterpret_cast<SetHandler>( tier0->FindSymbol( "Plat_SetWatchdogHandlerFunction", nullptr ) );
	auto begin = reinterpret_cast<Begin>( tier0->FindSymbol( "Plat_BeginWatchdogTimer", nullptr ) );
	auto end = reinterpret_cast<End>( tier0->FindSymbol( "Plat_EndWatchdogTimer", nullptr ) );
	check( setHandler && begin && end, "watchdog exports present" );
	static volatile sig_atomic_t fired = 0;
	auto waitMs = []( unsigned ms ) {
		timespec start{}, now{};
		clock_gettime( CLOCK_MONOTONIC, &start );
		do
		{
			usleep( 1000 );
			clock_gettime( CLOCK_MONOTONIC, &now );
		} while ( ( now.tv_sec - start.tv_sec ) * 1000 + ( now.tv_nsec - start.tv_nsec ) / 1000000 < ms );
	};
	if ( setHandler && begin && end )
	{
		setHandler( [] { fired = fired + 1; } );
		begin( 1 );
		waitMs( 400 );
		check( fired == 0, "the watchdog does not fire early" );
		waitMs( 1100 );
		check( fired == 1, "the watchdog fires its handler once" );
		end();
		begin( 1 );
		waitMs( 300 );
		end();
		waitMs( 1200 );
		check( fired == 1, "an ended watchdog does not fire" );
	}

	// T5: stack capture through GetCallStack.
	typedef int ( *GetStack )( void **, int, int );
	auto getStack = reinterpret_cast<GetStack>( tier0->FindSymbol( "GetCallStack", nullptr ) );
	check( getStack != nullptr, "GetCallStack present" );
	if ( getStack != nullptr )
	{
		void *frames[16] = {};
		const int n = getStack( frames, 16, 0 );
		check( n >= 2 && n <= 16 && frames[0] != nullptr, "GetCallStack returns frames" );
		check( getStack( frames, 0, 0 ) == 0, "GetCallStack with no room returns none" );
	}
	printf( "CONFORMANCE %d %d\n", checks, failures );
	fflush( stdout );
	_Exit( failures ? 1 : 0 );
}
