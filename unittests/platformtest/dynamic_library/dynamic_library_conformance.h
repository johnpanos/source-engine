//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared outcome oracle for every provider of the RFC 0001 loader
//          contract. Native providers must supply real fixture libraries;
//          deterministic providers certify contract behavior only.
//
//=============================================================================//

#ifndef PLATFORMTEST_DYNAMIC_LIBRARY_CONFORMANCE_H
#define PLATFORMTEST_DYNAMIC_LIBRARY_CONFORMANCE_H

#include "platform/contracts/dynamic_library.h"

#include <cstdio>
#include <type_traits>
#include <utility>

namespace platformtest
{

struct DynLibFixture
{
	const char *validPath;
	const char *validSymbol;
	void *expectedSymbolAddr;
	const char *missingPath;
	const char *missingSymbol;
};

struct ConformanceReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define PT_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

// Start with a failure so a provider cannot pass by leaving the output untouched.
inline platform::DynamicLibraryError PreviousFailure()
{
	return { platform::DynamicLibraryOp::kUnload, platform::DynamicLibraryStatus::kProviderError,
	    123, "previous request" };
}

inline void CheckLibrarySymbols(
    ConformanceReport &r, platform::IDynamicLibrary &library, const DynLibFixture &fx )
{
	using platform::DynamicLibraryOp;
	using platform::DynamicLibraryStatus;

	auto error = PreviousFailure();
	void *symbol = library.FindSymbol( fx.validSymbol, &error );
	PT_CHECK( r, symbol != nullptr );
	PT_CHECK( r, error.IsOk() );
	if ( fx.expectedSymbolAddr != nullptr )
	{
		PT_CHECK( r, symbol == fx.expectedSymbolAddr );
	}
	PT_CHECK( r, library.FindSymbol( fx.validSymbol, nullptr ) == symbol );

	error = PreviousFailure();
	PT_CHECK( r, library.FindSymbol( fx.missingSymbol, &error ) == nullptr );
	PT_CHECK( r, error.status == DynamicLibraryStatus::kSymbolNotFound );
	PT_CHECK( r, error.operation == DynamicLibraryOp::kFindSymbol );
	PT_CHECK( r, error.requested == fx.missingSymbol );
	PT_CHECK( r, library.FindSymbol( fx.missingSymbol, nullptr ) == nullptr );

	const char *invalidNames[] = { nullptr, "" };
	for ( const char *name : invalidNames )
	{
		error = PreviousFailure();
		PT_CHECK( r, library.FindSymbol( name, &error ) == nullptr );
		PT_CHECK( r, error.status == DynamicLibraryStatus::kInvalidArgument );
		PT_CHECK( r, error.operation == DynamicLibraryOp::kFindSymbol );
		PT_CHECK( r, error.requested == name );
		PT_CHECK( r, library.FindSymbol( name, nullptr ) == nullptr );
	}

	// Failure is local to that request and cannot poison subsequent lookups.
	PT_CHECK( r, library.FindSymbol( fx.validSymbol, &error ) == symbol );
	PT_CHECK( r, error.IsOk() );
}

inline void CheckNoLoad(
    ConformanceReport &r, platform::IDynamicLibraryLoader &loader, const DynLibFixture &fx )
{
	using platform::DynamicLibraryOp;
	using platform::DynamicLibraryStatus;
	const int countBefore = loader.LiveLibraryCount();
	const bool supported = loader.SupportsNoLoad();
	const char *paths[] = { fx.validPath, fx.missingPath, nullptr, "" };
	for ( const char *path : paths )
	{
		auto error = PreviousFailure();
		const bool resolved = loader.TryResolveNoLoad( path, &error );
		const bool expected = supported && path == fx.validPath;
		PT_CHECK( r, resolved == expected );
		PT_CHECK( r, loader.LiveLibraryCount() == countBefore );
		if ( !supported )
		{
			PT_CHECK( r, error.status == DynamicLibraryStatus::kUnsupportedNoLoad );
		}
		else if ( path == fx.validPath )
		{
			PT_CHECK( r, error.IsOk() );
		}
		else if ( path == fx.missingPath )
		{
			PT_CHECK( r, error.status == DynamicLibraryStatus::kNotFound );
		}
		else
		{
			PT_CHECK( r, error.status == DynamicLibraryStatus::kInvalidArgument );
		}
		if ( !expected )
		{
			PT_CHECK( r, error.operation == DynamicLibraryOp::kResolveNoLoad );
			PT_CHECK( r, error.requested == path );
		}
		PT_CHECK( r, loader.TryResolveNoLoad( path, nullptr ) == expected );
		PT_CHECK( r, loader.LiveLibraryCount() == countBefore );
	}
	PT_CHECK( r, loader.SupportsNoLoad() == supported );
}

inline ConformanceReport RunDynamicLibraryConformance(
    platform::IDynamicLibraryLoader &loader, const DynLibFixture &fx )
{
	using platform::DynamicLibraryOp;
	using platform::DynamicLibraryStatus;
	ConformanceReport r;
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );
	CheckNoLoad( r, loader, fx );

	auto error = PreviousFailure();
	platform::IDynamicLibrary *first = loader.Load( fx.validPath, &error );
	PT_CHECK( r, first != nullptr );
	PT_CHECK( r, error.IsOk() );
	PT_CHECK( r, loader.LiveLibraryCount() == 1 );
	if ( first != nullptr )
	{
		CheckLibrarySymbols( r, *first, fx );
	}
	CheckNoLoad( r, loader, fx );
	loader.Unload( nullptr );
	PT_CHECK( r, loader.LiveLibraryCount() == 1 );

	// Duplicate requests own separate releases, even if the OS reuses a handle.
	error = PreviousFailure();
	platform::IDynamicLibrary *second = loader.Load( fx.validPath, &error );
	PT_CHECK( r, second != nullptr );
	PT_CHECK( r, second != first );
	PT_CHECK( r, error.IsOk() );
	PT_CHECK( r, loader.LiveLibraryCount() == 2 );
	if ( first != nullptr )
	{
		loader.Unload( first );
	}
	PT_CHECK( r, loader.LiveLibraryCount() == 1 );
	// Do not dereference the known-invalid pointer from a broken aliasing provider.
	if ( second != nullptr && second != first )
	{
		CheckLibrarySymbols( r, *second, fx );
	}

	const char *invalidPaths[] = { fx.missingPath, nullptr, "" };
	for ( const char *path : invalidPaths )
	{
		error = PreviousFailure();
		platform::IDynamicLibrary *unexpected = loader.Load( path, &error );
		PT_CHECK( r, unexpected == nullptr );
		PT_CHECK( r,
		    error.status == ( path == fx.missingPath ? DynamicLibraryStatus::kNotFound
		                                             : DynamicLibraryStatus::kInvalidArgument ) );
		PT_CHECK( r, error.operation == DynamicLibraryOp::kLoad );
		PT_CHECK( r, error.requested == path );
		if ( unexpected != nullptr )
		{
			loader.Unload( unexpected );
		}
		unexpected = loader.Load( path, nullptr );
		PT_CHECK( r, unexpected == nullptr );
		if ( unexpected != nullptr )
		{
			loader.Unload( unexpected );
		}
		PT_CHECK( r, loader.LiveLibraryCount() == 1 );
	}
	if ( second != nullptr && second != first )
	{
		loader.Unload( second );
	}
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );

	// Clean second instance, absent error output, and failure-then-success.
	platform::IDynamicLibrary *again = loader.Load( fx.validPath, nullptr );
	PT_CHECK( r, again != nullptr );
	PT_CHECK( r, loader.LiveLibraryCount() == 1 );
	if ( again != nullptr )
	{
		CheckLibrarySymbols( r, *again, fx );
		loader.Unload( again );
	}
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );
	CheckNoLoad( r, loader, fx );
	return r;
}

// Independent compositions must not consume each other's libraries. Keeping the
// foreign library alive makes this a legal input, with no dangling-pointer test.
inline ConformanceReport RunDynamicLibraryIsolationConformance(
    platform::IDynamicLibraryLoader &first, platform::IDynamicLibraryLoader &second,
    const DynLibFixture &fx )
{
	ConformanceReport r;
	PT_CHECK( r, first.LiveLibraryCount() == 0 );
	PT_CHECK( r, second.LiveLibraryCount() == 0 );
	auto *firstLibrary = first.Load( fx.validPath, nullptr );
	auto *secondLibrary = second.Load( fx.validPath, nullptr );
	PT_CHECK( r, firstLibrary != nullptr );
	PT_CHECK( r, secondLibrary != nullptr );
	PT_CHECK( r, firstLibrary != secondLibrary );
	first.Unload( secondLibrary );
	second.Unload( firstLibrary );
	PT_CHECK( r, first.LiveLibraryCount() == 1 );
	PT_CHECK( r, second.LiveLibraryCount() == 1 );
	// Only use survivors if the observable ownership state stayed valid.
	if ( firstLibrary != nullptr && first.LiveLibraryCount() == 1 )
	{
		CheckLibrarySymbols( r, *firstLibrary, fx );
		first.Unload( firstLibrary );
	}
	PT_CHECK( r, first.LiveLibraryCount() == 0 );
	PT_CHECK( r, second.LiveLibraryCount() == 1 );
	if ( secondLibrary != nullptr && second.LiveLibraryCount() == 1 )
	{
		CheckLibrarySymbols( r, *secondLibrary, fx );
		second.Unload( secondLibrary );
	}
	PT_CHECK( r, second.LiveLibraryCount() == 0 );
	return r;
}

// Scoped ownership (platform::LoadedLibrary): each value releases exactly its
// own library, on scope exit, reset, and move-assignment over a live value.
static_assert( !std::is_copy_constructible_v<platform::LoadedLibrary> );
static_assert( !std::is_copy_assignable_v<platform::LoadedLibrary> );
static_assert( std::is_nothrow_move_constructible_v<platform::LoadedLibrary> );
static_assert( std::is_nothrow_move_assignable_v<platform::LoadedLibrary> );

inline ConformanceReport RunScopedLibraryConformance(
    platform::IDynamicLibraryLoader &loader, const DynLibFixture &fx )
{
	using platform::DynamicLibraryOp;
	using platform::DynamicLibraryStatus;
	using platform::LoadedLibrary;
	ConformanceReport r;
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );
	{
		auto loaded = platform::LoadScoped( loader, fx.validPath );
		PT_CHECK( r, loaded.HasValue() );
		PT_CHECK( r, loader.LiveLibraryCount() == 1 );
		if ( loaded )
		{
			PT_CHECK( r, static_cast<bool>( loaded.Value() ) );
			PT_CHECK( r, loaded.Value().FindSymbol( fx.validSymbol ) != nullptr );
		}
	}
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );

	auto missing = platform::LoadScoped( loader, fx.missingPath );
	PT_CHECK( r, !missing.HasValue() );
	if ( !missing )
	{
		PT_CHECK( r, missing.Error().status == DynamicLibraryStatus::kNotFound );
		PT_CHECK( r, missing.Error().operation == DynamicLibraryOp::kLoad );
		PT_CHECK( r, missing.Error().requested == fx.missingPath );
	}
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );

	LoadedLibrary empty;
	platform::DynamicLibraryError error = PreviousFailure();
	PT_CHECK( r, !empty && empty.Get() == nullptr );
	PT_CHECK( r, empty.FindSymbol( fx.validSymbol, &error ) == nullptr );
	PT_CHECK( r, error.status == DynamicLibraryStatus::kInvalidArgument );
	empty.Reset();

	LoadedLibrary first( loader, loader.Load( fx.validPath, nullptr ) );
	LoadedLibrary second( loader, loader.Load( fx.validPath, nullptr ) );
	PT_CHECK( r, first && second && loader.LiveLibraryCount() == 2 );
	// A provider that hands both requests one library is caught here; the
	// release sequence below would then free a library still held, so it is
	// only exercised on distinct ownership.
	const bool distinct = first.Get() != second.Get();
	PT_CHECK( r, distinct );
	LoadedLibrary moved( std::move( first ) );
	PT_CHECK( r, !first && moved && loader.LiveLibraryCount() == 2 );
	if ( distinct )
	{
		moved = std::move( second ); // releases the library moved held
		PT_CHECK( r, !second && moved && loader.LiveLibraryCount() == 1 );
		LoadedLibrary &alias = moved;
		moved = std::move( alias ); // self-move keeps ownership
		PT_CHECK( r, moved && loader.LiveLibraryCount() == 1 );
		if ( loader.LiveLibraryCount() == 1 )
		{
			PT_CHECK( r, moved.FindSymbol( fx.validSymbol ) != nullptr );
		}
		moved.Reset();
		PT_CHECK( r, !moved && loader.LiveLibraryCount() == 0 );
		moved.Reset();
		PT_CHECK( r, loader.LiveLibraryCount() == 0 );
	}
	else
	{
		// Release the one aliased library once and drop the second reference.
		moved.Reset();
		static_cast<void>( second.Release() );
	}

	// Release hands ownership back to the caller without unloading.
	LoadedLibrary held( loader, loader.Load( fx.validPath, nullptr ) );
	platform::IDynamicLibrary *released = held.Release();
	PT_CHECK( r, released != nullptr && !held && loader.LiveLibraryCount() == 1 );
	held.Reset();
	PT_CHECK( r, loader.LiveLibraryCount() == 1 );
	loader.Unload( released );
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );

	// A value built from a failed load is empty and releases nothing.
	LoadedLibrary failed( loader, loader.Load( fx.missingPath, nullptr ) );
	PT_CHECK( r, !failed && loader.LiveLibraryCount() == 0 );
	return r;
}

inline int ReportConformance( const char *suiteName, const ConformanceReport &r )
{
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", suiteName, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", suiteName, r.checks );
	return 0;
}

inline int RunPositive(
    const char *suiteName, platform::IDynamicLibraryLoader &loader, const DynLibFixture &fx )
{
	return ReportConformance( suiteName, RunDynamicLibraryConformance( loader, fx ) );
}

} // namespace platformtest

#endif // PLATFORMTEST_DYNAMIC_LIBRARY_CONFORMANCE_H
