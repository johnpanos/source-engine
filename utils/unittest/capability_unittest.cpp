//========= Copyright Valve Corporation, All rights reserved. ============//
#include "testing/runner.h"
#include <cstdio>

testing::TestResult RunCompositionConformance();
testing::TestResult RunExpectedConformance();
testing::TestResult RunRunnerConformance();

namespace
{
class ConsoleReport final : public testing::ITestReport
{
public:
	void Result( std::string_view name, testing::TestResult result ) override
	{
		std::printf( "%.*s: %zu checks, %zu failures, %zu skips\n", static_cast<int>( name.size() ),
		    name.data(), result.checks, result.failures, result.skipped );
		checks += result.checks;
		failures += result.failures + result.skipped + ( result.checks == 0 ? 1 : 0 );
		++executed;
	}
	void Error( std::string_view message ) override
	{
		std::fprintf(
		    stderr, "FAIL unittest: %.*s\n", static_cast<int>( message.size() ), message.data() );
		++failures;
	}
	std::size_t checks = 0, failures = 0, executed = 0;
};
}

int main()
{
	const testing::Suite suites[] = {
	    { "foundation.expected", RunExpectedConformance },
	    { "platform.composition", RunCompositionConformance },
	    { "platform.test-runner", RunRunnerConformance },
	};
	ConsoleReport report;
	const int result = testing::RunLinkedSuites( suites, report );
	if ( report.executed != std::size( suites ) )
		++report.failures;
	std::printf( "CONFORMANCE %zu %zu\n", report.checks, report.failures );
	return result || report.failures ? 1 : 0;
}
