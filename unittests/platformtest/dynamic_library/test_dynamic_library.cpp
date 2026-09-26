//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for the RFC 0001 dynamic-library loading
//			capability (PLAT-DYNLIB-001, Q-FOUNDATION). Runs the shared suite
//			against the deterministic test backend on the linux-headless-core
//			profile (include root "public", C++20, no tier0/GPU/display).
//
//			This certifies the CONTRACT semantics via a fake provider. It is NOT
//			evidence of OS loader behavior; a native Win32/POSIX provider runs
//			the same shared suite against a real fixture library to certify that.
//
//			Build/run: tools/quality/conformance.py check --suite platform.dynamic_library
//
//=============================================================================//

#include "dynamic_library_conformance.h"
#include "fake_dynamic_library.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

// Two real functions whose addresses serve as fixture symbols. Using distinct
// functions lets the suite assert an exact expected address.
int FixtureCreateInterface()
{
	return 42;
}
int FixtureGetTickCount()
{
	return 7;
}

using platformtest::DynLibFixture;
using platformtest::FakeLibraryDef;

std::vector<FakeLibraryDef> MakeDefs()
{
	std::vector<FakeLibraryDef> defs;
	FakeLibraryDef engine;
	engine.path = "fixtures/engine.so";
	engine.symbols = {
	    { "CreateInterface", reinterpret_cast<void *>( &FixtureCreateInterface ) },
	    { "GetTickCount", reinterpret_cast<void *>( &FixtureGetTickCount ) },
	};
	defs.push_back( std::move( engine ) );
	return defs;
}

DynLibFixture MakeFixture()
{
	DynLibFixture fx;
	fx.validPath = "fixtures/engine.so";
	fx.validSymbol = "CreateInterface";
	fx.expectedSymbolAddr = reinterpret_cast<void *>( &FixtureCreateInterface );
	fx.missingPath = "fixtures/does_not_exist.so";
	fx.missingSymbol = "NoSuchSymbol";
	return fx;
}

} // namespace

int main()
{
	const DynLibFixture fx = MakeFixture();
	int rc = 0;
	int checks = 0;
	int failures = 0;

	// Variant 1: a provider that does NOT support no-load -- the suite verifies
	// the unsupported optional capability is reported explicitly.
	{
		platformtest::CFakeDynamicLibraryLoader loader( MakeDefs(), /*supportsNoLoad=*/false );
		const platformtest::ConformanceReport r =
		    platformtest::RunDynamicLibraryConformance( loader, fx );
		checks += r.checks;
		failures += r.failures;
		rc |= platformtest::ReportConformance( "test_dynamic_library[no-load unsupported]", r );
	}

	// Variant 2: a provider that DOES support no-load -- the suite verifies the
	// resolve-without-load path succeeds for a valid path.
	{
		platformtest::CFakeDynamicLibraryLoader loader( MakeDefs(), /*supportsNoLoad=*/true );
		const platformtest::ConformanceReport r =
		    platformtest::RunDynamicLibraryConformance( loader, fx );
		checks += r.checks;
		failures += r.failures;
		rc |= platformtest::ReportConformance( "test_dynamic_library[no-load supported]", r );
	}
	{
		platformtest::CFakeDynamicLibraryLoader loader( MakeDefs(), /*supportsNoLoad=*/false );
		const platformtest::ConformanceReport r =
		    platformtest::RunScopedLibraryConformance( loader, fx );
		checks += r.checks;
		failures += r.failures;
		rc |= platformtest::ReportConformance( "test_dynamic_library[scoped ownership]", r );
	}
	{
		platformtest::CFakeDynamicLibraryLoader first( MakeDefs(), false );
		platformtest::CFakeDynamicLibraryLoader second( MakeDefs(), true );
		const platformtest::ConformanceReport r =
		    platformtest::RunDynamicLibraryIsolationConformance( first, second, fx );
		checks += r.checks;
		failures += r.failures;
		rc |= platformtest::ReportConformance( "test_dynamic_library[independent loaders]", r );
	}

	if ( rc == 0 )
	{
		std::printf( "ok test_dynamic_library: all variants passed\n" );
	}
	return testing::ReportConformance( checks, failures );
}
