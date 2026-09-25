//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: foundation.scoped-resource conformance (RFC 0006 M1 / roadmap R05).
//
//=============================================================================//

#include "foundation/scoped_resource.h"
#include "testing/checks.h"

#include <cstdint>
#include <cstdio>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
std::vector<int> g_Closed;

struct CountedTraits
{
	using Handle = int;
	static constexpr int Invalid() noexcept { return -1; }
	static void Close( int handle ) noexcept { g_Closed.push_back( handle ); }
};
using Counted = foundation::ScopedResource<CountedTraits>;

struct ZeroInvalidTraits
{
	using Handle = std::uint64_t;
	static constexpr std::uint64_t Invalid() noexcept { return 0; }
	static void Close( std::uint64_t ) noexcept {}
};

static_assert( sizeof( Counted ) == sizeof( int ) );
static_assert( !std::is_copy_constructible_v<Counted> && !std::is_copy_assignable_v<Counted> );
static_assert( std::is_nothrow_move_constructible_v<Counted> );
static_assert( std::is_nothrow_move_assignable_v<Counted> );
static_assert( !std::is_convertible_v<int, Counted> );
static_assert( sizeof( foundation::ScopedResource<ZeroInvalidTraits> ) == sizeof( std::uint64_t ) );
} // namespace

testing::TestResult RunScopedResourceConformance()
{
	testing::Checks checks;
	auto check = [&]( bool value, std::source_location where = std::source_location::current() )
	{
		checks.That( value, "check", where );
	};
	auto closed = []( std::vector<int> expected )
	{
		return g_Closed == expected;
	};

	g_Closed.clear();
	{
		Counted empty;
		check( !empty && empty.Get() == -1 );
	}
	check( closed( {} ) ); // the invalid handle is never closed

	{
		Counted a( 3 );
		check( a && a.Get() == 3 );
	}
	check( closed( { 3 } ) ); // destruction closes once

	g_Closed.clear();
	{
		Counted a( 4 );
		Counted b( std::move( a ) );
		check( !a && b.Get() == 4 );
		Counted c( 5 );
		c = std::move( b ); // closes 5, takes 4
		check( !b && c.Get() == 4 && closed( { 5 } ) );
		Counted &self = c;
		c = std::move( self ); // self-move keeps the handle
		check( c.Get() == 4 && closed( { 5 } ) );
	}
	check( closed( { 5, 4 } ) );

	g_Closed.clear();
	{
		Counted a( 6 );
		a.Reset( 6 ); // resetting to the owned handle keeps it open
		check( a.Get() == 6 && closed( {} ) );
		a.Reset( 7 );
		check( a.Get() == 7 && closed( { 6 } ) );
		a.Reset();
		check( !a && closed( { 6, 7 } ) );
		a.Reset( 8 );
		const int released = a.Release();
		check( released == 8 && !a );
	}
	check( closed( { 6, 7 } ) ); // a released handle is the caller's

	foundation::ScopedResource<ZeroInvalidTraits> zero;
	check( !zero && zero.Get() == 0 );
	zero.Reset( 42 );
	check( zero && zero.Get() == 42 );

	return checks.Result();
}

#ifndef SOURCE_CONFORMANCE_LINKED
int main()
{
	const auto result = RunScopedResourceConformance();
	std::printf( "CONFORMANCE %zu %zu\n", result.checks, result.failures );
	return result.Passed() ? 0 : 1;
}
#endif
