//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Common check matchers for strict C++20 conformance suites
//          (RFC 0001 rank 2 "common test matchers", roadmap R05). One place
//          counts checks, reports where a check failed and what it saw, and
//          produces the checks-v1 record the shared runner reads.
//
//=============================================================================//

#ifndef TESTING_CHECKS_H
#define TESTING_CHECKS_H

#include "testing/conformance_result.h"
#include "testing/runner.h"

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>

namespace testing
{

namespace detail
{
template <typename T>
concept Streamable = requires( std::ostream &stream, const T &value ) { stream << value; };

template <typename T> std::string Describe( const T &value )
{
	if constexpr ( Streamable<T> )
	{
		std::ostringstream stream;
		stream << value;
		return stream.str();
	}
	else
	{
		return "<unprintable>";
	}
}
} // namespace detail

// Counts checks and failures for one suite. A failed check prints one line,
// "FAIL <file>:<line>: <what>", with the observed values where the matcher has
// them. Checks never throw or abort, so a suite always reaches its record.
class Checks
{
public:
	explicit Checks( std::FILE *out = stdout ) noexcept : m_Out( out ) {}

	bool That( bool condition, std::string_view what,
	    std::source_location where = std::source_location::current() )
	{
		return Record( condition, what, where, {} );
	}

	template <typename Actual, typename Expected>
	bool Equal( const Actual &actual, const Expected &expected, std::string_view what,
	    std::source_location where = std::source_location::current() )
	{
		const bool equal = actual == expected;
		return Record( equal, what, where,
		    equal ? std::string()
		          : "actual " + detail::Describe( actual ) + ", expected " +
		                detail::Describe( expected ) );
	}

	// |actual - expected| <= tolerance; a NaN on either side never matches.
	bool Near( double actual, double expected, double tolerance, std::string_view what,
	    std::source_location where = std::source_location::current() )
	{
		const bool near = std::fabs( actual - expected ) <= tolerance;
		return Record( near, what, where,
		    near ? std::string()
		         : "actual " + detail::Describe( actual ) + ", expected " +
		               detail::Describe( expected ) + " +/- " + detail::Describe( tolerance ) );
	}

	std::size_t Count() const noexcept { return m_Checks; }
	std::size_t Failures() const noexcept { return m_Failures; }

	// The result a linked runner consumes; zero checks never passes.
	TestResult Result() const noexcept { return { m_Checks, m_Failures, 0 }; }

	// Prints the checks-v1 record and returns the process exit status.
	int Report() const { return ReportConformance( m_Checks, m_Failures ); }

private:
	bool Record( bool passed, std::string_view what, const std::source_location &where,
	    const std::string &observed )
	{
		++m_Checks;
		if ( passed )
			return true;
		++m_Failures;
		std::fprintf( m_Out, "FAIL %s:%u: %.*s%s%s\n", where.file_name(),
		    static_cast<unsigned>( where.line() ), static_cast<int>( what.size() ), what.data(),
		    observed.empty() ? "" : " (", observed.empty() ? "" : ( observed + ")" ).c_str() );
		return false;
	}

	std::FILE *m_Out;
	std::size_t m_Checks = 0;
	std::size_t m_Failures = 0;
};

} // namespace testing

#endif // TESTING_CHECKS_H
