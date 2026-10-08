// R103 T3 oracle: Tier 0's command line with nobody calling Plat_SetCommandLine
// must equal the legacy read of /proc/self/cmdline (every NUL, the last
// included, turned into a space). The host re-executes itself with known
// arguments; no Tier 0 headers.
#include "platform/contracts/dynamic_library.h"
#include "platform/posix/dynamic_library_provider.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
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
			printf( "usage: cmdline_oracle <libtier0.so>\nCONFORMANCE 1 1\n" );
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
	printf( "CONFORMANCE %d %d\n", checks, failures );
	fflush( stdout );
	_Exit( failures ? 1 : 0 );
}
