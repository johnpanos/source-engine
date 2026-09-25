//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: testing.checks conformance: the shared matchers count, locate and
//          describe failures, and zero checks never pass (RFC 0001 rank 2).
//
//=============================================================================//

#include "testing/checks.h"

#include <cmath>
#include <cstdio>
#include <string>

namespace
{
struct Unprintable
{
	int value;
	bool operator==( const Unprintable & ) const = default;
};

// Runs `body` against a private Checks writing to a temporary file and returns
// everything it printed.
template <typename Body> std::string Captured( testing::Checks *&subject, Body body )
{
	std::FILE *file = std::tmpfile();
	if ( !file )
		return "<no tmpfile>";
	testing::Checks checks( file );
	subject = &checks;
	body( checks );
	std::string text;
	std::rewind( file );
	for ( int ch; ( ch = std::fgetc( file ) ) != EOF; )
		text.push_back( static_cast<char>( ch ) );
	std::fclose( file );
	return text;
}
} // namespace

int main()
{
	testing::Checks checks;
	testing::Checks *inner = nullptr;
	std::size_t innerChecks = 0, innerFailures = 0;
	auto keep = [&]( testing::Checks &c )
	{
		innerChecks = c.Count();
		innerFailures = c.Failures();
	};

	std::string out = Captured( inner,
	    [&]( testing::Checks &c )
	    {
		    c.That( true, "holds" );
		    c.Equal( 3, 3, "equal ints" );
		    c.Near( 1.0, 1.0 + 1e-9, 1e-6, "near doubles" );
		    keep( c );
	    } );
	checks.Equal( out, std::string(), "passing checks print nothing" );
	checks.Equal( innerChecks, std::size_t( 3 ), "passing checks are counted" );
	checks.Equal( innerFailures, std::size_t( 0 ), "no failures counted" );

	out = Captured( inner,
	    [&]( testing::Checks &c )
	    {
		    c.That( false, "fails plainly" );
		    keep( c );
	    } );
	checks.That( out.find( "FAIL " ) == 0 && out.find( "test_checks.cpp:" ) != std::string::npos &&
	                 out.find( "fails plainly" ) != std::string::npos,
	    "a failure names its file, line and description" );
	checks.Equal( innerFailures, std::size_t( 1 ), "a failure is counted" );

	out = Captured( inner,
	    [&]( testing::Checks &c )
	    {
		    c.Equal( 2, 5, "ints differ" );
	    } );
	checks.That( out.find( "actual 2, expected 5" ) != std::string::npos,
	    "Equal reports actual and expected values" );

	out = Captured( inner,
	    [&]( testing::Checks &c )
	    {
		    c.Equal( Unprintable{ 1 }, Unprintable{ 2 }, "structs" );
	    } );
	checks.That( out.find( "<unprintable>" ) != std::string::npos,
	    "a type without operator<< is described, not rejected" );

	out = Captured( inner,
	    [&]( testing::Checks &c )
	    {
		    c.Near( 1.0, 1.1, 0.05, "outside tolerance" );
		    c.Near( std::nan( "" ), 0.0, 1.0, "nan never matches" );
		    keep( c );
	    } );
	checks.Equal( innerFailures, std::size_t( 2 ), "Near rejects out-of-tolerance and NaN" );
	checks.That( out.find( "+/- 0.05" ) != std::string::npos, "Near reports its tolerance" );

	testing::Checks empty( stdout );
	checks.That( !empty.Result().Passed(), "zero checks never pass" );
	std::FILE *sink = std::tmpfile();
	testing::Checks quiet( sink ? sink : stdout );
	quiet.That( false, "one failure" );
	checks.That( !quiet.Result().Passed(), "one failure fails the result" );
	if ( sink )
		std::fclose( sink );

	return checks.Report();
}
