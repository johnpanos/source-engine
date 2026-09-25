//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: foundation.strong-id conformance (RFC 0006 M1 / roadmap R05).
//
//=============================================================================//

#include "foundation/strong_id.h"
#include "testing/runner.h"

#include <cstdint>
#include <cstdio>
#include <type_traits>
#include <unordered_map>

namespace
{
struct AlphaTag;
struct BetaTag;
struct SignedTag;
using AlphaId = foundation::StrongId<AlphaTag, std::uint32_t>;
using BetaId = foundation::StrongId<BetaTag, std::uint32_t>;
using SignedId = foundation::StrongId<SignedTag, int, -1>;

// Layout and type rules are compile-time contract clauses.
static_assert( sizeof( AlphaId ) == sizeof( std::uint32_t ) );
static_assert( alignof( AlphaId ) == alignof( std::uint32_t ) );
static_assert( std::is_trivially_copyable_v<AlphaId> );
static_assert( std::is_standard_layout_v<AlphaId> );
static_assert( !std::is_convertible_v<std::uint32_t, AlphaId> );
static_assert( !std::is_convertible_v<AlphaId, std::uint32_t> );
static_assert( !std::is_convertible_v<AlphaId, BetaId> );
static_assert( !std::is_same_v<AlphaId, BetaId> );
static_assert( !AlphaId{}.IsValid() && AlphaId{ 3 }.IsValid() );
static_assert( SignedId::kInvalid == -1 && !SignedId{}.IsValid() && SignedId{ 0 }.IsValid() );
} // namespace

testing::TestResult RunStrongIdConformance()
{
	int failures = 0, checks = 0;
	auto check = [&]( bool value )
	{
		++checks;
		if ( !value )
			++failures;
	};

	const AlphaId none;
	check( none.value == 0 && !none.IsValid() );
	const AlphaId one{ 1 }, two{ 2 };
	check( one.IsValid() && one.value == 1 );
	check( one == AlphaId{ 1 } && one != two );
	check( one < two && two > one && ( one <=> two ) < 0 );

	const SignedId unset;
	check( unset.value == -1 && !unset.IsValid() );
	check( SignedId{ 0 }.IsValid() && SignedId{ 0 } != unset );

	std::unordered_map<AlphaId, int> byId;
	byId[one] = 10;
	byId[two] = 20;
	byId[AlphaId{ 1 }] += 1;
	check( byId.size() == 2 && byId[one] == 11 && byId[two] == 20 );
	check( std::hash<AlphaId>{}( one ) == std::hash<std::uint32_t>{}( 1u ) );

	AlphaId assigned;
	assigned = two;
	check( assigned == two );

	return { static_cast<std::size_t>( checks ), static_cast<std::size_t>( failures ), 0 };
}

#ifndef SOURCE_CONFORMANCE_LINKED
int main()
{
	const auto result = RunStrongIdConformance();
	std::printf( "CONFORMANCE %zu %zu\n", result.checks, result.failures );
	return result.Passed() ? 0 : 1;
}
#endif
