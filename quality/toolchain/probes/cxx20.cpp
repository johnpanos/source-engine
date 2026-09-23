//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Target-level C++20 compiler and standard-library probe for the
//          cxx20 dialect (RFC 0006 M0). Waf configure compiles, links and runs
//          it with each configuration's real flags and defines; the boundary
//          verifier runs it per compiler. Missing required support fails at
//          compile time; optional facilities are reported, never assumed.
//
//=============================================================================//

#include <version>

#include <atomic>
#include <bit>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <thread>

#if __cplusplus < 202002L
#error "cxx20 probe: __cplusplus is below 202002L"
#endif
#if !defined( __cpp_concepts ) || __cpp_concepts < 201907L
#error "cxx20 probe: concepts unavailable"
#endif
#if !defined( __cpp_designated_initializers )
#error "cxx20 probe: designated initializers unavailable"
#endif
#if !defined( __cpp_impl_three_way_comparison )
#error "cxx20 probe: three-way comparison unavailable"
#endif
#if !defined( __cpp_consteval )
#error "cxx20 probe: consteval unavailable"
#endif
#if !defined( __cpp_lib_concepts )
#error "cxx20 probe: <concepts> library unavailable"
#endif
#if !defined( __cpp_lib_span )
#error "cxx20 probe: std::span unavailable"
#endif
#if !defined( __cpp_lib_atomic_wait )
#error "cxx20 probe: std::atomic wait/notify unavailable"
#endif
#if !defined( __cpp_lib_bit_cast )
#error "cxx20 probe: std::bit_cast unavailable"
#endif

template <class T>
requires std::integral<T>
consteval T Twice( T value )
{
	return value * 2;
}

struct Version
{
	int major;
	int minor;
	auto operator<=>( const Version & ) const = default;
};

[[nodiscard( "probe result" )]] static int SumSpan( std::span<const int> values )
{
	int total = 0;
	for ( int value : values )
		total += value;
	return total;
}

int main()
{
	const int values[] = { 1, 2, 3, Twice( 2 ) };
	if ( SumSpan( values ) != 10 )
		return 1;

	constexpr Version older{ .major = 1, .minor = 2 };
	constexpr Version newer{ .major = 1, .minor = 3 };
	static_assert( older < newer );

	if ( std::bit_cast<std::uint32_t>( 1.0f ) != 0x3f800000u )
		return 2;

	// Atomic wait/notify with a real second thread: the waiter observes the
	// published value through the atomic, not through the notification.
	std::atomic<int> state{ 0 };
	std::thread producer( [&state] {
		state.store( 1, std::memory_order_release );
		state.notify_one();
	} );
	state.wait( 0, std::memory_order_acquire );
	producer.join();
	if ( state.load( std::memory_order_acquire ) != 1 )
		return 3;

	std::optional<std::string_view> name = std::string_view( "cxx20" );
	auto owned = std::make_unique<int>( static_cast<int>( name->size() ) );

#if defined( __cpp_lib_jthread )
	const int jthread = 1;
#else
	const int jthread = 0;
#endif
#if defined( __cpp_lib_source_location )
	const int sourceLocation = 1;
#else
	const int sourceLocation = 0;
#endif
	std::printf( "cxx20-probe ok __cplusplus=%ld jthread=%d source_location=%d\n",
	    static_cast<long>( __cplusplus ), jthread, sourceLocation );
	return *owned == 5 ? 0 : 4;
}
