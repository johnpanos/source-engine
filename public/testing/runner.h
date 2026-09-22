//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef TESTING_RUNNER_H
#define TESTING_RUNNER_H

#include <cstddef>
#include <span>
#include <string_view>

namespace testing
{
struct TestResult
{
	std::size_t checks = 0;
	std::size_t failures = 0;
	std::size_t skipped = 0;
	bool Passed() const noexcept { return checks > 0 && failures == 0 && skipped == 0; }
};

struct Suite
{
	std::string_view name;
	TestResult ( *run )();
};

class ITestReport
{
public:
	virtual ~ITestReport() = default;
	virtual void Result( std::string_view name, TestResult result ) = 0;
	virtual void Error( std::string_view message ) = 0;
};

// Suites and reporter outlive this synchronous call. Suites may not throw.
// Zero discovery, missing callbacks, duplicates, skips and zero checks fail.
// All state/providers are per invocation; no ambient registration or loader.
[[nodiscard]] int RunLinkedSuites( std::span<const Suite> suites, ITestReport &report );
}
#endif
