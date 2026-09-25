//========= Copyright Valve Corporation, All rights reserved. ============//
#include "foundation/expected.h"
#include "testing/checks.h"
#include <cstdio>
#include <memory>
#include <string>
#include <type_traits>

namespace
{
struct Resource
{
	int *live;
	explicit Resource( int &count ) noexcept : live( &count ) { ++*live; }
	Resource( Resource &&other ) noexcept : live( std::exchange( other.live, nullptr ) ) {}
	Resource( const Resource & ) = delete;
	~Resource()
	{
		if ( live )
			--*live;
	}
};
struct NoDefault
{
	explicit NoDefault( int value ) : value( value ) {}
	int value;
};
}

testing::TestResult RunExpectedConformance()
{
	using foundation::Expected;
	using foundation::MakeUnexpected;
	testing::Checks checks;
	int live = 0;
	auto check = [&]( bool value, std::source_location where = std::source_location::current() )
	{
		checks.That( value, "check", where );
	};
	static_assert( !std::is_copy_constructible_v<Expected<Resource, int>> );
	static_assert( std::is_nothrow_move_constructible_v<Expected<Resource, int>> );
	{
		Expected<Resource, int> value{ Resource( live ) };
		check( value.HasValue() && live == 1 );
		Expected<Resource, int> moved( std::move( value ) );
		check( moved.HasValue() && live == 1 );
		moved = MakeUnexpected( 42 );
		check( !moved && moved.Error() == 42 && live == 0 );
		moved = Resource( live );
		check( moved && live == 1 );
		auto *same = &moved;
		moved = std::move( *same );
		check( moved && live == 1 );
	}
	check( live == 0 );
	Expected<std::string, std::string> text( std::string( "ok" ) );
	Expected<std::string, std::string> error( MakeUnexpected( std::string( "error" ) ) );
	text = error;
	check( !text && text.Error() == "error" && text.ValueOr( "fallback" ) == "fallback" );
	error = std::string( "restored" );
	text = error;
	check( text && text.Value() == "restored" );
	Expected<void, NoDefault> success;
	check( !!success );
	success = MakeUnexpected( NoDefault( 7 ) );
	check( !success && success.Error().value == 7 );
	success = Expected<void, NoDefault>::Ok();
	check( !!success );
	Expected<void, std::unique_ptr<int>> uniqueError(
	    MakeUnexpected( std::make_unique<int>( 19 ) ) );
	auto transferred = std::move( uniqueError );
	check( !transferred && *transferred.Error() == 19 );
	return checks.Result();
}

#ifndef SOURCE_CONFORMANCE_LINKED
int main()
{
	const auto result = RunExpectedConformance();
	std::printf( "CONFORMANCE %zu %zu\n", result.checks, result.failures );
	return result.Passed() ? 0 : 1;
}
#endif
