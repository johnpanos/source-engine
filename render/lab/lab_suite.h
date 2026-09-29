//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's suite driver (RFC 0016 K11): named check outcomes,
//			the seeded programs a suite's sensitivity run must catch, and the
//			command line every suite shares:
//
//			render_lab suite <name> [--validate] [--verbose]
//			    [--seeded <defect>] [--sensitivity]
//
//			A plain run reports its checks; --seeded runs them with one
//			seeded program; --sensitivity runs the control and every seeded
//			program and passes when the control passes and each seeded
//			program fails the checks its defect breaks. Each prints one
//			checks-v1 record.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_SUITE_H
#define RENDER_LAB_LAB_SUITE_H

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace render::lab
{

// A check's outcome, named so the sensitivity runs can say which failed.
struct Outcome
{
	std::string name;
	bool passed = false;
	std::string detail;
};

class Results
{
public:
	void That( bool condition, std::string name, std::string detail = {} )
	{
		m_Outcomes.push_back( { std::move( name ), condition, std::move( detail ) } );
	}
	const std::vector<Outcome> &Outcomes() const { return m_Outcomes; }
	// Whether a check whose name starts with `prefix` failed.
	bool Failed( std::string_view prefix ) const;
	std::size_t FailureCount() const;

private:
	std::vector<Outcome> m_Outcomes;
};

// A seeded program: its SPIR-V words and the check prefix it must fail.
struct Seeded
{
	const char *name;
	std::span<const std::uint32_t> module;
	const char *breaks;
};

// One run of a suite's checks, with a replacement fragment module (empty for
// the program's own); `messages` receives the validation message count.
using SuiteRun = std::function<std::optional<std::string>( bool validate,
    std::span<const std::uint32_t> module, Results &results, std::uint64_t &messages )>;

int RunSeededSuite(
    int argc, char **argv, const char *suite, std::span<const Seeded> seeded, const SuiteRun &run );

} // namespace render::lab

#endif // RENDER_LAB_LAB_SUITE_H
