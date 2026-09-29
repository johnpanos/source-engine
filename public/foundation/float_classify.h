//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: NaN and finiteness tests that survive the products' -ffast-math.
//
//          Release engine targets build with -ffast-math, which implies
//          -ffinite-math-only. Under it GCC and clang fold std::isnan and
//          isnan to false and std::isfinite to true, and compile a == b
//          without a parity check, so a NaN compares equal to everything
//          and x != x is false. These tests read the IEEE 754 bit pattern
//          instead, so they give the IEEE answer under any flags.
//
//          The argument is taken by reference: under -ffinite-math-only
//          clang marks by-value float parameters nofpclass(nan inf) and may
//          assume the test's answer. The empty asm hides the bits from the
//          optimizers' own "this is an FP class test" recognition.
//
//          Limit: a value that fast-math arithmetic in the same function
//          produced may already be assumed finite by clang. The tests are
//          reliable for values that were stored, loaded or passed in.
//
//          Header-only; dialect-neutral (C++11 and later).
//          unittests/mathlibtest/fastmath_nan_conformance.cpp checks them
//          with the products' flags, against the naive forms.
//
//=============================================================================//

#ifndef FOUNDATION_FLOAT_CLASSIFY_H
#define FOUNDATION_FLOAT_CLASSIFY_H

#include <cstdint>

// No <cstring> on GCC/clang: legacy code includes this after tier1 headers
// that redefine strncpy and friends.
#if defined( __GNUC__ ) || defined( __clang__ )
#define FOUNDATION_FLOAT_CLASSIFY_COPY( dst, src )                                                 \
	__builtin_memcpy( &( dst ), &( src ), sizeof( dst ) )
#define FOUNDATION_FLOAT_CLASSIFY_OPAQUE( bits ) __asm__( "" : "+r"( bits ) )
#else
#include <cstring>
#define FOUNDATION_FLOAT_CLASSIFY_COPY( dst, src ) std::memcpy( &( dst ), &( src ), sizeof( dst ) )
#define FOUNDATION_FLOAT_CLASSIFY_OPAQUE( bits ) ( (void)0 )
#endif

namespace foundation
{
namespace detail
{

inline std::uint32_t FloatBits( const float &value )
{
	static_assert( sizeof( float ) == sizeof( std::uint32_t ), "IEEE 754 binary32 float" );
	std::uint32_t bits;
	FOUNDATION_FLOAT_CLASSIFY_COPY( bits, value );
	FOUNDATION_FLOAT_CLASSIFY_OPAQUE( bits );
	return bits;
}

inline std::uint64_t DoubleBits( const double &value )
{
	static_assert( sizeof( double ) == sizeof( std::uint64_t ), "IEEE 754 binary64 double" );
	std::uint64_t bits;
	FOUNDATION_FLOAT_CLASSIFY_COPY( bits, value );
	FOUNDATION_FLOAT_CLASSIFY_OPAQUE( bits );
	return bits;
}

} // namespace detail

// True for any NaN (quiet or signalling, either sign).
inline bool IsNaN( const float &value )
{
	return ( detail::FloatBits( value ) & 0x7FFFFFFFu ) > 0x7F800000u;
}

inline bool IsNaN( const double &value )
{
	return ( detail::DoubleBits( value ) & 0x7FFFFFFFFFFFFFFFull ) > 0x7FF0000000000000ull;
}

// False for NaN and for either infinity.
inline bool IsFinite( const float &value )
{
	return ( detail::FloatBits( value ) & 0x7F800000u ) != 0x7F800000u;
}

inline bool IsFinite( const double &value )
{
	return ( detail::DoubleBits( value ) & 0x7FF0000000000000ull ) != 0x7FF0000000000000ull;
}

} // namespace foundation

#undef FOUNDATION_FLOAT_CLASSIFY_COPY
#undef FOUNDATION_FLOAT_CLASSIFY_OPAQUE

#endif // FOUNDATION_FLOAT_CLASSIFY_H
