//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity (negative-provider) check for the RFC 0001 dynamic-library
//			conformance suite (PLAT-DYNLIB-001, Q-FOUNDATION). A conformance suite
//			that never rejects anything is worthless. This test feeds the SAME
//			shared predicate a set of deliberately-broken loaders -- each
//			violating exactly one contract clause -- and asserts the predicate
//			catches every one, while the conforming test backend passes.
//
//			This suite PASSES (exit 0) when the oracle correctly distinguishes
//			conforming from broken providers.
//
//			Build/run: tools/quality/conformance.py check --suite platform.dynamic_library.sensitivity
//
//=============================================================================//

#include "dynamic_library_conformance.h"
#include "fake_dynamic_library.h"

#include "platform/contracts/dynamic_library.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace
{

int FixtureSymbol()
{
	return 1;
}

const char *const kValidPath = "fixtures/engine.so";
const char *const kValidSymbol = "CreateInterface";

// Which single clause a broken loader violates.
enum class Defect
{
	kNone,
	kUnloadDoesNotRelease,     // Unload keeps the handle live
	kLoadMissingSucceeds,      // a missing path "loads"
	kSilentNoLoad,             // no-load unsupported but silently returns true
	kMissingSymbolReturnsAddr, // an absent symbol resolves to an address
	kLoadLeavesStaleError,
	kDuplicateAliasesOwnership,
	kNullUnloadReleasesOwned,
	kForeignUnloadReleasesOwned,
	kNoLoadAcquiresLibrary,
	kWrongErrorOperation,
};

class CBrokenLibrary : public platform::IDynamicLibrary
{
public:
	explicit CBrokenLibrary( bool missingSymbolReturnsAddr )
	    : m_missingSymbolReturnsAddr( missingSymbolReturnsAddr )
	{
	}

	void *FindSymbol( const char *name, platform::DynamicLibraryError *error ) override
	{
		if ( name == nullptr || name[0] == '\0' )
		{
			if ( error != nullptr )
			{
				*error = { platform::DynamicLibraryOp::kFindSymbol,
				    platform::DynamicLibraryStatus::kInvalidArgument, 0, name };
			}
			return nullptr;
		}
		void *addr = reinterpret_cast<void *>( &FixtureSymbol );
		if ( name != nullptr && std::strcmp( name, kValidSymbol ) == 0 )
		{
			if ( error != nullptr )
			{
				*error = platform::DynamicLibraryError{};
			}
			return addr;
		}
		if ( m_missingSymbolReturnsAddr )
		{
			// DEFECT: an absent symbol resolves anyway.
			if ( error != nullptr )
			{
				*error = platform::DynamicLibraryError{};
			}
			return addr;
		}
		if ( error != nullptr )
		{
			*error = { platform::DynamicLibraryOp::kFindSymbol,
			    platform::DynamicLibraryStatus::kSymbolNotFound, 0, name };
		}
		return nullptr;
	}

private:
	bool m_missingSymbolReturnsAddr;
};

// A loader that behaves correctly except for one injected defect. Its destructor
// always frees every allocated library, so the negative test never crashes or
// leaks regardless of the defect being exercised.
class CBrokenLoader : public platform::IDynamicLibraryLoader
{
public:
	explicit CBrokenLoader( Defect defect ) : m_defect( defect ) {}

	~CBrokenLoader() override
	{
		for ( platform::IDynamicLibrary *lib : m_live )
		{
			delete lib;
		}
	}

	platform::IDynamicLibrary *Load(
	    const char *path, platform::DynamicLibraryError *error ) override
	{
		if ( path == nullptr || path[0] == '\0' )
		{
			if ( error != nullptr )
			{
				error->operation = m_defect == Defect::kWrongErrorOperation
				                       ? platform::DynamicLibraryOp::kFindSymbol
				                       : platform::DynamicLibraryOp::kLoad;
				error->status = platform::DynamicLibraryStatus::kInvalidArgument;
				error->requested = path;
			}
			return nullptr;
		}

		const bool isValid = std::strcmp( path, kValidPath ) == 0;
		if ( !isValid && m_defect != Defect::kLoadMissingSucceeds )
		{
			if ( error != nullptr )
			{
				error->operation = platform::DynamicLibraryOp::kLoad;
				error->status = platform::DynamicLibraryStatus::kNotFound;
				error->requested = path;
			}
			return nullptr;
		}

		// Either a valid path, or a missing path under the missing-load defect.
		if ( error != nullptr && m_defect != Defect::kLoadLeavesStaleError )
		{
			*error = platform::DynamicLibraryError{};
		}
		if ( m_defect == Defect::kDuplicateAliasesOwnership && !m_live.empty() )
		{
			return m_live.front();
		}
		platform::IDynamicLibrary *lib =
		    new CBrokenLibrary( m_defect == Defect::kMissingSymbolReturnsAddr );
		m_live.push_back( lib );
		return lib;
	}

	void Unload( platform::IDynamicLibrary *library ) override
	{
		if ( library == nullptr && m_defect == Defect::kNullUnloadReleasesOwned && !m_live.empty() )
		{
			delete m_live.back();
			m_live.pop_back();
			return;
		}
		if ( m_defect == Defect::kUnloadDoesNotRelease )
		{
			// DEFECT: keep the handle live (still freed in the destructor).
			return;
		}
		for ( std::size_t i = 0; i < m_live.size(); ++i )
		{
			if ( m_live[i] == library )
			{
				delete m_live[i];
				m_live.erase( m_live.begin() + static_cast<std::ptrdiff_t>( i ) );
				return;
			}
		}
		if ( library != nullptr && m_defect == Defect::kForeignUnloadReleasesOwned &&
		     !m_live.empty() )
		{
			delete m_live.back();
			m_live.pop_back();
		}
	}

	int LiveLibraryCount() const override { return static_cast<int>( m_live.size() ); }

	bool SupportsNoLoad() const override { return false; }

	bool TryResolveNoLoad( const char *path, platform::DynamicLibraryError *error ) override
	{
		if ( m_defect == Defect::kNoLoadAcquiresLibrary )
		{
			m_live.push_back( new CBrokenLibrary( false ) );
		}
		if ( m_defect == Defect::kSilentNoLoad )
		{
			// DEFECT: unsupported optional capability silently emulated.
			(void)path;
			if ( error != nullptr )
			{
				*error = platform::DynamicLibraryError{};
			}
			return true;
		}
		if ( error != nullptr )
		{
			error->operation = platform::DynamicLibraryOp::kResolveNoLoad;
			error->status = platform::DynamicLibraryStatus::kUnsupportedNoLoad;
			error->requested = path;
		}
		return false;
	}

private:
	Defect m_defect;
	std::vector<platform::IDynamicLibrary *> m_live;
};

platformtest::DynLibFixture MakeFixture()
{
	platformtest::DynLibFixture fx;
	fx.validPath = kValidPath;
	fx.validSymbol = kValidSymbol;
	fx.expectedSymbolAddr = nullptr; // broken lib returns its own address
	fx.missingPath = "fixtures/does_not_exist.so";
	fx.missingSymbol = "NoSuchSymbol";
	return fx;
}

std::vector<platformtest::FakeLibraryDef> MakeConformingDefs()
{
	std::vector<platformtest::FakeLibraryDef> defs;
	platformtest::FakeLibraryDef engine;
	engine.path = kValidPath;
	engine.symbols = {
	    { kValidSymbol, reinterpret_cast<void *>( &FixtureSymbol ) },
	};
	defs.push_back( std::move( engine ) );
	return defs;
}

struct Case
{
	Defect defect;
	const char *name;
};

} // namespace

int main()
{
	const platformtest::DynLibFixture fx = MakeFixture();
	int checks = 0;
	int failures = 0;

	// 1) The conforming test backend must PASS the shared predicate. If it does
	//    not, the suite is over-strict and cannot be trusted.
	{
		platformtest::CFakeDynamicLibraryLoader good( MakeConformingDefs(), /*noLoad=*/false );
		platformtest::ConformanceReport r = platformtest::RunDynamicLibraryConformance( good, fx );
		++checks;
		if ( r.failures != 0 )
		{
			std::printf( "FAIL: conforming test backend rejected by suite "
			             "(%d/%d); first: %s (line %d)\n",
			    r.failures, r.checks, r.firstFailure, r.firstFailureLine );
			++failures;
		}
	}
	{
		CBrokenLoader control( Defect::kNone );
		++checks;
		failures += platformtest::ReportConformance( "sensitivity[defects disabled]",
		    platformtest::RunDynamicLibraryConformance( control, fx ) );
	}

	// 2) Every broken loader must be CAUGHT (failures > 0). If any slips through,
	//    the corresponding clause is not actually enforced.
	const Case cases[] = {
	    { Defect::kUnloadDoesNotRelease, "unload-does-not-release" },
	    { Defect::kLoadMissingSucceeds, "load-missing-succeeds" },
	    { Defect::kSilentNoLoad, "silent-no-load" },
	    { Defect::kMissingSymbolReturnsAddr, "missing-symbol-returns-addr" },
	    { Defect::kLoadLeavesStaleError, "load-leaves-stale-error" },
	    { Defect::kDuplicateAliasesOwnership, "duplicate-aliases-ownership" },
	    { Defect::kNullUnloadReleasesOwned, "null-unload-releases-owned" },
	    { Defect::kNoLoadAcquiresLibrary, "no-load-acquires-library" },
	    { Defect::kWrongErrorOperation, "wrong-error-operation" },
	};
	for ( const Case &c : cases )
	{
		CBrokenLoader bad( c.defect );
		platformtest::ConformanceReport r = platformtest::RunDynamicLibraryConformance( bad, fx );
		++checks;
		if ( r.failures == 0 )
		{
			std::printf( "FAIL: broken provider '%s' was NOT caught by the suite "
			             "(suite is vacuous for that clause)\n",
			    c.name );
			++failures;
		}
	}
	{
		CBrokenLoader first( Defect::kNone );
		CBrokenLoader second( Defect::kNone );
		++checks;
		failures += platformtest::ReportConformance( "sensitivity[isolation defects disabled]",
		    platformtest::RunDynamicLibraryIsolationConformance( first, second, fx ) );
	}
	{
		CBrokenLoader first( Defect::kForeignUnloadReleasesOwned );
		CBrokenLoader second( Defect::kNone );
		const auto report =
		    platformtest::RunDynamicLibraryIsolationConformance( first, second, fx );
		++checks;
		if ( report.failures == 0 )
		{
			std::printf( "FAIL: foreign-unload-releases-owned was not caught\n" );
			++failures;
		}
	}

	// 3) The scoped-ownership oracle accepts the control and catches providers
	//    whose release or failure reporting LoadedLibrary depends on.
	{
		CBrokenLoader control( Defect::kNone );
		++checks;
		failures += platformtest::ReportConformance( "sensitivity[scoped defects disabled]",
		    platformtest::RunScopedLibraryConformance( control, fx ) );
	}
	const Case scopedCases[] = {
	    { Defect::kUnloadDoesNotRelease, "scoped:unload-does-not-release" },
	    { Defect::kLoadMissingSucceeds, "scoped:load-missing-succeeds" },
	    { Defect::kDuplicateAliasesOwnership, "scoped:duplicate-aliases-ownership" },
	};
	for ( const Case &c : scopedCases )
	{
		CBrokenLoader bad( c.defect );
		const auto report = platformtest::RunScopedLibraryConformance( bad, fx );
		++checks;
		if ( report.failures == 0 )
		{
			std::printf(
			    "FAIL: broken provider '%s' was NOT caught by the scoped oracle\n", c.name );
			++failures;
		}
	}

	if ( failures == 0 )
	{
		std::printf( "ok test_dynamic_library_negative: suite accepts conforming "
		             "and rejects all %zu broken providers\n",
		    1 + sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
