//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity for the POSIX loader's lifetime rule: destroying the
//          loader while it still owns a library must abort (expected crash),
//          never leave that library's symbols dangling silently.
//
//=============================================================================//

#include "../../../platform/posix/dynamic_library_provider.h"

#include <cstdio>

namespace
{
struct NoTelemetry final : platform::IDynamicLibraryObserver
{
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

int main()
{
	NoTelemetry observer;
	auto loader = platform::CreatePosixDynamicLibraryLoader( observer );
	platform::IDynamicLibrary *library = loader->Load( CONFORMANCE_SHARED_FIXTURE, nullptr );
	std::printf( "loaded: %s\n", library != nullptr ? "yes" : "no" );
	loader.reset(); // must abort
	std::printf( "loader destroyed with a live library and did not abort\n" );
	return library != nullptr ? 0 : 1;
}
