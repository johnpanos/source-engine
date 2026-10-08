//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for the RFC 0001 process-environment
//			capability (PLAT-ENV-001, Q-FOUNDATION). Runs the shared suite against
//			the deterministic backend, with and without a program name.
//
//			Certifies contract semantics via a fake; NOT evidence of native
//			argv/environ behavior.
//
//			Build/run: tools/quality/conformance.py check --suite platform.process_environment
//
//=============================================================================//

#include "fake_process_environment.h"
#include "process_environment_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

void RunVariant( const char *name, const platform::IProcessEnvironment &env,
    const platformtest::ProcessEnvironmentFixture &fixture, int &checks, int &failures )
{
	const platformtest::ProcessEnvironmentReport r =
	    platformtest::RunProcessEnvironmentConformance( env, fixture );
	checks += r.checks;
	failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", name, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return;
	}
	std::printf( "ok %s: %d checks passed\n", name, r.checks );
}

platformtest::ProcessEnvironmentFixture Fixture( const char *const *args, int count )
{
	platformtest::ProcessEnvironmentFixture f;
	f.arguments = args;
	f.argumentCount = count;
	f.presentName = "SOURCE_TEST_VAR";
	f.presentValue = "caf\xc3\xa9 = 1";
	f.emptyName = "SOURCE_TEST_EMPTY";
	f.absentName = "SOURCE_TEST_ABSENT";
	f.caseVariantName = "source_test_var";
	return f;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	const std::map<std::string, std::string> vars = {
	    { "SOURCE_TEST_VAR", "caf\xc3\xa9 = 1" },
	    { "SOURCE_TEST_EMPTY", "" },
	    { "PATH", "/usr/bin" },
	};
	{
		const char *const args[] = { "/opt/game/portal", "-game", "portal", "", "+map x" };
		platformtest::CFakeProcessEnvironment env( { args, args + 5 }, vars );
		RunVariant( "test_process_environment[argv]", env, Fixture( args, 5 ), checks, failures );
	}
	{
		// A mobile container may supply no arguments at all.
		platformtest::CFakeProcessEnvironment env( {}, vars, 7, platform::DebuggerState::kUnknown );
		RunVariant(
		    "test_process_environment[no-args]", env, Fixture( nullptr, 0 ), checks, failures );
	}
	return testing::ReportConformance( checks, failures );
}
