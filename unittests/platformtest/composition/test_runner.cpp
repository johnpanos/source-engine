//========= Copyright Valve Corporation, All rights reserved. ============//
#include "testing/runner.h"
#include <cstdio>

namespace
{
class Report final : public testing::ITestReport
{
public:
	void Result( std::string_view, testing::TestResult ) override { ++results; }
	void Error( std::string_view ) override { ++errors; }
	int results = 0, errors = 0;
};
testing::TestResult Pass() { return { 1, 0, 0 }; }
testing::TestResult Fail() { return { 1, 1, 0 }; }
testing::TestResult Empty() { return {}; }
testing::TestResult Skip() { return { 1, 0, 1 }; }
}

testing::TestResult RunRunnerConformance()
{
	testing::TestResult result;
	auto check = [&]( bool value ) { ++result.checks; if ( !value ) ++result.failures; };
	for ( int repeat = 0; repeat < 2; ++repeat )
	{
		Report report;
		const testing::Suite suites[] = { { "first", Pass }, { "second", Pass } };
		check( testing::RunLinkedSuites( suites, report ) == 0 && report.results == 2 && report.errors == 0 );
	}
	for ( auto bad : { Fail, Empty, Skip } )
	{
		Report report;
		const testing::Suite suites[] = { { "bad", bad }, { "still-required", Pass } };
		check( testing::RunLinkedSuites( suites, report ) != 0 && report.results == 2 );
	}
	{
		Report report;
		check( testing::RunLinkedSuites( {}, report ) != 0 && report.results == 0 && report.errors == 1 );
	}
	for ( auto bad : { testing::Suite{ "missing", nullptr }, testing::Suite{ "", Pass } } )
	{
		Report report;
		const testing::Suite suites[] = { bad };
		check( testing::RunLinkedSuites( suites, report ) != 0 && report.results == 0 && report.errors == 1 );
	}
	{
		Report report;
		const testing::Suite suites[] = { { "duplicate", Pass }, { "duplicate", Pass } };
		check( testing::RunLinkedSuites( suites, report ) != 0 && report.results == 0 );
	}
	return result;
}

#ifndef SOURCE_CONFORMANCE_LINKED
int main()
{
	const auto result = RunRunnerConformance();
	std::printf( "CONFORMANCE %zu %zu\n", result.checks, result.failures );
	return result.Passed() ? 0 : 1;
}
#endif
