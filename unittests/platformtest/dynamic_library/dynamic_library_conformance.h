//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the RFC 0001 dynamic-library loading
//			capability (platform::IDynamicLibraryLoader). Every provider that
//			claims the contract -- the deterministic test backend here, and the
//			real Win32/POSIX backends when they land -- runs THIS predicate.
//
//			The suite is provider-agnostic: it drives an IDynamicLibraryLoader&
//			through the behavior RFC 0001 requires and reports structured
//			failures. A fake provider certifies contract semantics; native
//			providers additionally certify OS behavior (RFC 0001: fake-provider
//			success is not evidence of OS behavior). A negative provider proves
//			this predicate is not vacuous -- see test_dynamic_library_negative.
//
//=============================================================================//

#ifndef PLATFORMTEST_DYNAMIC_LIBRARY_CONFORMANCE_H
#define PLATFORMTEST_DYNAMIC_LIBRARY_CONFORMANCE_H

#include "platform/contracts/dynamic_library.h"

#include <cstdio>

namespace platformtest
{

// Description of a known-good fixture the provider must be able to serve. A fake
// backend is seeded with these; a native backend points them at a real fixture
// shared library built with the suite.
struct DynLibFixture
{
	const char *validPath;      // a path the loader can open
	const char *validSymbol;    // a symbol exported by validPath
	void *expectedSymbolAddr;   // if non-null, FindSymbol(validSymbol) must equal it
	const char *missingPath;    // a path the loader must fail to open
	const char *missingSymbol;  // a symbol absent from validPath
};

// Accumulates results and remembers the first failure so the runner can print a
// single first-divergence line.
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

// Runs the full contract against `loader` using `fx`. Returns the report; the
// caller decides pass/fail (failures == 0 for a conforming provider). The suite
// does not print per-check success; a positive test asserts failures == 0 and a
// negative test asserts failures > 0 for a broken provider.
inline ConformanceReport RunDynamicLibraryConformance(
	platform::IDynamicLibraryLoader &loader, const DynLibFixture &fx )
{
	using platform::DynamicLibraryError;
	using platform::DynamicLibraryOp;
	using platform::DynamicLibraryStatus;

	ConformanceReport r;

	// A fresh loader owns nothing.
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );

	// --- Load of a valid path succeeds and takes ownership -------------------
	DynamicLibraryError loadErr;
	platform::IDynamicLibrary *lib = loader.Load( fx.validPath, &loadErr );
	PT_CHECK( r, lib != nullptr );
	PT_CHECK( r, loadErr.IsOk() );
	PT_CHECK( r, loader.LiveLibraryCount() == 1 );

	if ( lib != nullptr )
	{
		// --- Known symbol resolves ------------------------------------------
		DynamicLibraryError symErr;
		void *sym = lib->FindSymbol( fx.validSymbol, &symErr );
		PT_CHECK( r, sym != nullptr );
		PT_CHECK( r, symErr.IsOk() );
		if ( fx.expectedSymbolAddr != nullptr )
		{
			PT_CHECK( r, sym == fx.expectedSymbolAddr );
		}

		// --- Missing symbol returns a structured failure --------------------
		DynamicLibraryError missSymErr;
		void *missSym = lib->FindSymbol( fx.missingSymbol, &missSymErr );
		PT_CHECK( r, missSym == nullptr );
		PT_CHECK( r, !missSymErr.IsOk() );
		PT_CHECK( r, missSymErr.status == DynamicLibraryStatus::kSymbolNotFound );
		PT_CHECK( r, missSymErr.operation == DynamicLibraryOp::kFindSymbol );
		PT_CHECK( r, missSymErr.requested == fx.missingSymbol );

		// --- Unload releases the handle -------------------------------------
		loader.Unload( lib );
		PT_CHECK( r, loader.LiveLibraryCount() == 0 );
	}

	// --- Missing library returns a structured failure ------------------------
	DynamicLibraryError missLibErr;
	platform::IDynamicLibrary *missLib = loader.Load( fx.missingPath, &missLibErr );
	PT_CHECK( r, missLib == nullptr );
	PT_CHECK( r, !missLibErr.IsOk() );
	PT_CHECK( r, missLibErr.status == DynamicLibraryStatus::kNotFound );
	PT_CHECK( r, missLibErr.operation == DynamicLibraryOp::kLoad );
	PT_CHECK( r, missLibErr.requested == fx.missingPath );
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );

	// --- Null argument is rejected explicitly, not crashed on ----------------
	DynamicLibraryError nullErr;
	platform::IDynamicLibrary *nullLib = loader.Load( nullptr, &nullErr );
	PT_CHECK( r, nullLib == nullptr );
	PT_CHECK( r, nullErr.status == DynamicLibraryStatus::kInvalidArgument );

	// --- Optional no-load capability is reported explicitly ------------------
	DynamicLibraryError noLoadErr;
	const bool resolved = loader.TryResolveNoLoad( fx.validPath, &noLoadErr );
	if ( loader.SupportsNoLoad() )
	{
		PT_CHECK( r, resolved );
		PT_CHECK( r, noLoadErr.IsOk() );
	}
	else
	{
		// Unsupported must be explicit: false + kUnsupportedNoLoad, never a
		// silent emulated success.
		PT_CHECK( r, !resolved );
		PT_CHECK( r, noLoadErr.status == DynamicLibraryStatus::kUnsupportedNoLoad );
		PT_CHECK( r, noLoadErr.operation == DynamicLibraryOp::kResolveNoLoad );
	}

	// --- A clean second load/unload cycle works (repeat-instance) ------------
	DynamicLibraryError reErr;
	platform::IDynamicLibrary *lib2 = loader.Load( fx.validPath, &reErr );
	PT_CHECK( r, lib2 != nullptr );
	PT_CHECK( r, loader.LiveLibraryCount() == 1 );
	if ( lib2 != nullptr )
	{
		loader.Unload( lib2 );
	}

	// --- Loader ends with no live handles (safe to destroy) ------------------
	PT_CHECK( r, loader.LiveLibraryCount() == 0 );

	return r;
}

// Convenience for a positive test: run the suite, print any first divergence,
// and return the process exit code (0 pass, 1 fail).
inline int RunPositive( const char *suiteName,
	platform::IDynamicLibraryLoader &loader, const DynLibFixture &fx )
{
	ConformanceReport r = RunDynamicLibraryConformance( loader, fx );
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n",
			suiteName, r.failures, r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", suiteName, r.checks );
	return 0;
}

} // namespace platformtest

#endif // PLATFORMTEST_DYNAMIC_LIBRARY_CONFORMANCE_H
