//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native conformance for the POSIX dynamic-library provider
//          (RFC 0001 rank 4 / roadmap R07). Runs the SAME shared loader,
//          isolation and scoped-ownership oracles as the deterministic test
//          backend, against a real fixture shared library that the runner
//          builds (CONFORMANCE_SHARED_FIXTURE), plus native-only clauses.
//
//=============================================================================//

#include "../dynamic_library/dynamic_library_conformance.h"
#include "../../../platform/posix/dynamic_library_provider.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

namespace
{

using platform::DynamicLibraryOp;
using platform::DynamicLibraryStatus;

const char kMissingPath[] = "/nonexistent-platform-fixture/libmissing.so";

// Records load-site telemetry (RFC 0001 rank 4) as the Sys_* bridge's Tier 0
// adapter will receive it.
struct RecordingObserver final : platform::IDynamicLibraryObserver
{
	struct Event
	{
		char kind; // 'L'oad, 'S'ymbol, 'U'nload
		const platform::IDynamicLibrary *library;
		DynamicLibraryStatus status;
		const void *symbol;
	};
	std::vector<Event> events;

	std::vector<std::string> resolved;

	void OnLoad( const char *, const char *resolvedPath, const platform::IDynamicLibrary *library,
	    const platform::DynamicLibraryError &result ) override
	{
		events.push_back( { 'L', library, result.status, nullptr } );
		resolved.push_back( resolvedPath != nullptr ? resolvedPath : "<null>" );
	}
	void OnFindSymbol( const platform::IDynamicLibrary *library, const char *, const void *symbol,
	    const platform::DynamicLibraryError &result ) override
	{
		events.push_back( { 'S', library, result.status, symbol } );
	}
	void OnUnload( const platform::IDynamicLibrary *library ) override
	{
		events.push_back( { 'U', library, DynamicLibraryStatus::kOk, nullptr } );
	}
};

platformtest::DynLibFixture MakeFixture()
{
	platformtest::DynLibFixture fx;
	fx.validPath = CONFORMANCE_SHARED_FIXTURE;
	fx.validSymbol = "PlatformFixtureAnswer";
	fx.expectedSymbolAddr = nullptr; // known only after load; called instead
	fx.missingPath = kMissingPath;
	fx.missingSymbol = "NoSuchSymbol";
	return fx;
}

// Clauses only a real loader can show.
platformtest::ConformanceReport RunNativeClauses( platform::IDynamicLibraryLoader &loader )
{
	platformtest::ConformanceReport r;

	// The symbol is the fixture's real function.
	{
		auto loaded = platform::LoadScoped( loader, CONFORMANCE_SHARED_FIXTURE );
		PT_CHECK( r, loaded.HasValue() );
		if ( loaded )
		{
			using AnswerFn = int ( * )();
			auto answer =
			    reinterpret_cast<AnswerFn>( loaded.Value().FindSymbol( "PlatformFixtureAnswer" ) );
			PT_CHECK( r, answer != nullptr );
			PT_CHECK( r, answer != nullptr && answer() == 42 );
		}
	}

	// A file that exists but is not a loadable library is a provider error,
	// not "not found".
	{
		platform::DynamicLibraryError error;
		PT_CHECK( r, loader.Load( "/proc/self/cmdline", &error ) == nullptr );
		PT_CHECK( r, error.status == DynamicLibraryStatus::kProviderError );
		PT_CHECK( r, error.operation == DynamicLibraryOp::kLoad );
	}

	// A bare name is opened in the working directory, never searched for: the
	// C library is always resolvable by the dynamic linker, but not here.
	{
		platform::DynamicLibraryError error;
		PT_CHECK( r, loader.Load( "libc.so.6", &error ) == nullptr );
		PT_CHECK( r, error.status == DynamicLibraryStatus::kNotFound );
		PT_CHECK( r, error.providerCode != 0 );
	}
	// ...and a bare name that does exist in the working directory is opened
	// from there. A provider that let dlopen search would miss it, because
	// the runner's build directory is not on the library path.
	{
		const std::string fixture = CONFORMANCE_SHARED_FIXTURE;
		const std::size_t slash = fixture.rfind( '/' );
		char previous[4096] = {};
		const bool saved = getcwd( previous, sizeof( previous ) ) != nullptr;
		PT_CHECK( r, saved && slash != std::string::npos );
		if ( saved && slash != std::string::npos &&
		     chdir( fixture.substr( 0, slash ).c_str() ) == 0 )
		{
			auto local = platform::LoadScoped( loader, fixture.c_str() + slash + 1 );
			PT_CHECK( r, local.HasValue() );
			PT_CHECK( r, local && local.Value().FindSymbol( "PlatformFixtureAnswer" ) != nullptr );
			PT_CHECK( r, chdir( previous ) == 0 );
		}
		else
		{
			PT_CHECK( r, false );
		}
	}
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );
	return r;
}

// Every native request reaches the observer: successes, failures, duplicate
// loads with distinct identities, symbol lookups, and unloads of exactly the
// loads they end. Null and foreign unloads are not native calls.
platformtest::ConformanceReport RunTelemetryClauses()
{
	platformtest::ConformanceReport r;
	RecordingObserver observer;
	auto loader = platform::CreatePosixDynamicLibraryLoader( observer );
	platform::IDynamicLibrary *first = loader->Load( CONFORMANCE_SHARED_FIXTURE, nullptr );
	platform::IDynamicLibrary *second = loader->Load( CONFORMANCE_SHARED_FIXTURE, nullptr );
	PT_CHECK( r, loader->Load( kMissingPath, nullptr ) == nullptr );
	PT_CHECK( r, loader->Load( nullptr, nullptr ) == nullptr );
	void *symbol =
	    first != nullptr ? first->FindSymbol( "PlatformFixtureAnswer", nullptr ) : nullptr;
	if ( second != nullptr )
	{
		PT_CHECK( r, second->FindSymbol( "NoSuchSymbol", nullptr ) == nullptr );
	}
	loader->Unload( nullptr );
	loader->Unload( second );
	loader->Unload( first );
	PT_CHECK( r, loader->LiveLibraryCount() == 0 );

	const auto &e = observer.events;
	PT_CHECK( r, e.size() == 8 );
	if ( e.size() == 8 )
	{
		PT_CHECK( r, e[0].kind == 'L' && e[0].library == first && first != nullptr );
		PT_CHECK( r, e[0].status == DynamicLibraryStatus::kOk );
		PT_CHECK( r, e[1].kind == 'L' && e[1].library == second && second != first );
		PT_CHECK( r, e[2].kind == 'L' && e[2].library == nullptr );
		PT_CHECK( r, e[2].status == DynamicLibraryStatus::kNotFound );
		PT_CHECK( r, e[3].kind == 'L' && e[3].status == DynamicLibraryStatus::kInvalidArgument );
		PT_CHECK( r, e[4].kind == 'S' && e[4].library == first && e[4].symbol == symbol );
		PT_CHECK( r, e[4].status == DynamicLibraryStatus::kOk && symbol != nullptr );
		PT_CHECK( r, e[5].kind == 'S' && e[5].library == second );
		PT_CHECK( r, e[5].status == DynamicLibraryStatus::kSymbolNotFound );
		PT_CHECK( r, e[6].kind == 'U' && e[6].library == second );
		PT_CHECK( r, e[7].kind == 'U' && e[7].library == first );
	}
	// Successful loads name the canonical file opened; failures name none.
	PT_CHECK( r, observer.resolved.size() == 4 );
	if ( observer.resolved.size() == 4 )
	{
		char canonical[4096];
		const bool known = realpath( CONFORMANCE_SHARED_FIXTURE, canonical ) != nullptr;
		PT_CHECK( r, known && observer.resolved[0] == canonical );
		PT_CHECK( r, known && observer.resolved[1] == canonical );
		PT_CHECK( r, observer.resolved[2] == "<null>" && observer.resolved[3] == "<null>" );
	}
	return r;
}

} // namespace

int main()
{
	const platformtest::DynLibFixture fx = MakeFixture();
	int checks = 0;
	int failures = 0;
	auto tally = [&]( const char *name, const platformtest::ConformanceReport &report )
	{
		checks += report.checks;
		failures += report.failures;
		platformtest::ReportConformance( name, report );
	};

	RecordingObserver observer;
	auto loader = platform::CreatePosixDynamicLibraryLoader( observer );
	tally( "posix[loader]", platformtest::RunDynamicLibraryConformance( *loader, fx ) );
	tally( "posix[scoped ownership]", platformtest::RunScopedLibraryConformance( *loader, fx ) );
	tally( "posix[native clauses]", RunNativeClauses( *loader ) );
	tally( "posix[load-site telemetry]", RunTelemetryClauses() );
	{
		RecordingObserver secondObserver;
		auto second = platform::CreatePosixDynamicLibraryLoader( secondObserver );
		tally( "posix[independent loaders]",
		    platformtest::RunDynamicLibraryIsolationConformance( *loader, *second, fx ) );
	}
	return testing::ReportConformance( checks, failures );
}
