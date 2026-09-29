//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's suite driver (RFC 0016 K11); see lab_suite.h.
//
//=============================================================================//

#include "lab_suite.h"

#include "testing/conformance_result.h"

#include <algorithm>
#include <cstdio>

namespace render::lab
{

bool Results::Failed( std::string_view prefix ) const
{
	for ( const Outcome &outcome : m_Outcomes )
	{
		if ( !outcome.passed && std::string_view( outcome.name ).starts_with( prefix ) )
			return true;
	}
	return false;
}

std::size_t Results::FailureCount() const
{
	return std::size_t( std::count_if( m_Outcomes.begin(), m_Outcomes.end(),
	    []( const Outcome &o )
	    {
		    return !o.passed;
	    } ) );
}

int RunSeededSuite( int argc, char **argv, const char *suite, std::span<const Seeded> seededSet,
    const SuiteRun &run )
{
	bool validate = false;
	bool sensitivity = false;
	bool verbose = false;
	std::string seeded;
	for ( int i = 0; i < argc; ++i )
	{
		const std::string arg = argv[i];
		if ( arg == "--validate" )
			validate = true;
		else if ( arg == "--verbose" )
			verbose = true;
		else if ( arg == "--sensitivity" )
			sensitivity = true;
		else if ( arg == "--seeded" && i + 1 < argc )
			seeded = argv[++i];
		else
		{
			std::fprintf( stderr, "render_lab suite %s: unknown option %s\n", suite, arg.c_str() );
			return 2;
		}
	}
	unsigned long checks = 0;
	unsigned long failures = 0;
	auto report = [&]( const Results &results )
	{
		for ( const Outcome &outcome : results.Outcomes() )
		{
			++checks;
			if ( verbose && outcome.passed )
				std::printf( "ok   %s%s%s\n", outcome.name.c_str(),
				    outcome.detail.empty() ? "" : ": ", outcome.detail.c_str() );
			if ( !outcome.passed )
			{
				++failures;
				std::fprintf( stderr, "FAIL %s%s%s\n", outcome.name.c_str(),
				    outcome.detail.empty() ? "" : ": ", outcome.detail.c_str() );
			}
		}
	};
	auto fail = [&]( const std::string &why )
	{
		std::fprintf( stderr, "FAIL render_lab %s: %s\n", suite, why.c_str() );
		return testing::ReportConformance( checks + 1, failures + 1 );
	};

	if ( !sensitivity )
	{
		std::span<const std::uint32_t> module;
		if ( !seeded.empty() )
		{
			const Seeded *found = nullptr;
			for ( const Seeded &s : seededSet )
				found = seeded == s.name ? &s : found;
			if ( !found )
				return fail( "no seeded defect " + seeded );
			module = found->module;
		}
		Results results;
		std::uint64_t messages = 0;
		if ( std::optional<std::string> why = run( validate, module, results, messages ) )
			return fail( *why );
		if ( validate )
			results.That( messages == 0, "validation.silent",
			    std::to_string( messages ) + " validation messages" );
		report( results );
		std::printf( "render_lab %s: %zu checks, %zu failed\n", suite, results.Outcomes().size(),
		    results.FailureCount() );
		return testing::ReportConformance( checks, failures );
	}

	// Sensitivity: the control passes, and each seeded program fails the
	// checks its defect breaks.
	Results control;
	std::uint64_t messages = 0;
	if ( std::optional<std::string> why = run( validate, {}, control, messages ) )
		return fail( *why );
	Results verdicts;
	verdicts.That( control.FailureCount() == 0, "sensitivity.control-passes",
	    std::to_string( control.FailureCount() ) + " control failures" );
	for ( const Seeded &s : seededSet )
	{
		Results results;
		if ( std::optional<std::string> why = run( validate, s.module, results, messages ) )
			return fail( *why );
		verdicts.That( results.Failed( s.breaks ), std::string( "sensitivity.detects." ) + s.name,
		    std::string( "no failure of " ) + s.breaks );
	}
	report( verdicts );
	return testing::ReportConformance( checks, failures );
}

} // namespace render::lab
