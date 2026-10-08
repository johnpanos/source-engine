//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SSE intrinsics the engine uses, in portable C++ for ARM
//			targets without NEON (the Nintendo 3DS's ARM11: ARMv6K, VFPv2,
//			no SIMD unit; RFC 0026). sse2neon.h needs ARMv7-A NEON or
//			ARMv8-A, so ssemath.h, vector4d.h and mathlib/sse.cpp take this
//			header when __ARM_NEON is absent. Types are GCC vector types,
//			which lower to scalar VFP code here; semantics follow Intel's
//			definitions (comparisons yield all-ones lanes, min/max return the
//			second operand when a lane is NaN, cvtss rounds to nearest).
//
//			Only the intrinsics the engine calls are defined; a new caller
//			that needs another fails to compile, by name.
//
//=============================================================================//

#ifndef SSE2SCALAR_H
#define SSE2SCALAR_H

#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float __m128 __attribute__( ( vector_size( 16 ), aligned( 16 ) ) );
typedef int32_t __m128i __attribute__( ( vector_size( 16 ), aligned( 16 ) ) );
typedef double __m128d __attribute__( ( vector_size( 16 ), aligned( 16 ) ) );
typedef int32_t __m64 __attribute__( ( vector_size( 8 ), aligned( 8 ) ) );

#define _MM_SHUFFLE( z, y, x, w ) ( ( ( z ) << 6 ) | ( ( y ) << 4 ) | ( ( x ) << 2 ) | ( w ) )
#define _MM_HINT_T0 1
#define _MM_HINT_T1 2
#define _MM_HINT_T2 3
#define _MM_HINT_NTA 0
#define _MM_FROUND_TO_NEAREST_INT 0x00
#define _MM_FROUND_TO_NEG_INF 0x01
#define _MM_FROUND_TO_POS_INF 0x02
#define _MM_FROUND_TO_ZERO 0x03
#define _MM_FROUND_CUR_DIRECTION 0x04
#define _MM_FROUND_NO_EXC 0x08

#define SSE2SCALAR_INLINE static inline __attribute__( ( always_inline ) )

// Bit casts between the vector types (no conversion).
template <typename To, typename From> SSE2SCALAR_INLINE To sse2scalar_bits( From from )
{
	static_assert( sizeof( To ) == sizeof( From ), "same size" );
	To to;
	memcpy( &to, &from, sizeof( to ) );
	return to;
}

// --- __m128: four floats -----------------------------------------------------

SSE2SCALAR_INLINE __m128 _mm_setzero_ps() { return __m128{ 0.0f, 0.0f, 0.0f, 0.0f }; }
SSE2SCALAR_INLINE __m128 _mm_set1_ps( float a ) { return __m128{ a, a, a, a }; }
SSE2SCALAR_INLINE __m128 _mm_set_ss( float a ) { return __m128{ a, 0.0f, 0.0f, 0.0f }; }
SSE2SCALAR_INLINE __m128 _mm_setr_ps( float a, float b, float c, float d ) { return __m128{ a, b, c, d }; }
SSE2SCALAR_INLINE __m128 _mm_load_ps( const float *p ) { return *(const __m128 *)p; }
SSE2SCALAR_INLINE __m128 _mm_loadu_ps( const float *p )
{
	__m128 r;
	memcpy( &r, p, sizeof( r ) );
	return r;
}
SSE2SCALAR_INLINE __m128 _mm_load_ss( const float *p ) { return __m128{ *p, 0.0f, 0.0f, 0.0f }; }
SSE2SCALAR_INLINE __m128 _mm_loadl_pi( __m128 a, const __m64 *p )
{
	float lo[2];
	memcpy( lo, p, sizeof( lo ) );
	return __m128{ lo[0], lo[1], a[2], a[3] };
}
SSE2SCALAR_INLINE void _mm_store_ps( float *p, __m128 a ) { *(__m128 *)p = a; }
SSE2SCALAR_INLINE void _mm_storeu_ps( float *p, __m128 a ) { memcpy( p, &a, sizeof( a ) ); }
SSE2SCALAR_INLINE void _mm_store_ss( float *p, __m128 a ) { *p = a[0]; }
// Non-temporal store: an ordinary aligned store (no write-combining here).
SSE2SCALAR_INLINE void _mm_stream_ps( float *p, __m128 a ) { *(__m128 *)p = a; }

SSE2SCALAR_INLINE __m128 _mm_add_ps( __m128 a, __m128 b ) { return a + b; }
SSE2SCALAR_INLINE __m128 _mm_sub_ps( __m128 a, __m128 b ) { return a - b; }
SSE2SCALAR_INLINE __m128 _mm_mul_ps( __m128 a, __m128 b ) { return a * b; }
SSE2SCALAR_INLINE __m128 _mm_div_ps( __m128 a, __m128 b ) { return a / b; }
SSE2SCALAR_INLINE __m128 _mm_add_ss( __m128 a, __m128 b ) { a[0] += b[0]; return a; }
SSE2SCALAR_INLINE __m128 _mm_sub_ss( __m128 a, __m128 b ) { a[0] -= b[0]; return a; }
SSE2SCALAR_INLINE __m128 _mm_mul_ss( __m128 a, __m128 b ) { a[0] *= b[0]; return a; }
SSE2SCALAR_INLINE __m128 _mm_sqrt_ps( __m128 a )
{
	return __m128{ sqrtf( a[0] ), sqrtf( a[1] ), sqrtf( a[2] ), sqrtf( a[3] ) };
}
SSE2SCALAR_INLINE __m128 _mm_sqrt_ss( __m128 a ) { a[0] = sqrtf( a[0] ); return a; }
SSE2SCALAR_INLINE __m128 _mm_rcp_ps( __m128 a ) { return 1.0f / a; }
SSE2SCALAR_INLINE __m128 _mm_rsqrt_ps( __m128 a ) { return 1.0f / _mm_sqrt_ps( a ); }
SSE2SCALAR_INLINE __m128 _mm_rsqrt_ss( __m128 a ) { a[0] = 1.0f / sqrtf( a[0] ); return a; }
SSE2SCALAR_INLINE __m128 _mm_max_ps( __m128 a, __m128 b )
{
	return __m128{ a[0] > b[0] ? a[0] : b[0], a[1] > b[1] ? a[1] : b[1], a[2] > b[2] ? a[2] : b[2],
		a[3] > b[3] ? a[3] : b[3] };
}
SSE2SCALAR_INLINE __m128 _mm_min_ps( __m128 a, __m128 b )
{
	return __m128{ a[0] < b[0] ? a[0] : b[0], a[1] < b[1] ? a[1] : b[1], a[2] < b[2] ? a[2] : b[2],
		a[3] < b[3] ? a[3] : b[3] };
}

SSE2SCALAR_INLINE __m128 _mm_and_ps( __m128 a, __m128 b )
{
	return sse2scalar_bits<__m128>( sse2scalar_bits<__m128i>( a ) & sse2scalar_bits<__m128i>( b ) );
}
SSE2SCALAR_INLINE __m128 _mm_or_ps( __m128 a, __m128 b )
{
	return sse2scalar_bits<__m128>( sse2scalar_bits<__m128i>( a ) | sse2scalar_bits<__m128i>( b ) );
}
SSE2SCALAR_INLINE __m128 _mm_xor_ps( __m128 a, __m128 b )
{
	return sse2scalar_bits<__m128>( sse2scalar_bits<__m128i>( a ) ^ sse2scalar_bits<__m128i>( b ) );
}
SSE2SCALAR_INLINE __m128 _mm_andnot_ps( __m128 a, __m128 b )
{
	return sse2scalar_bits<__m128>( ~sse2scalar_bits<__m128i>( a ) & sse2scalar_bits<__m128i>( b ) );
}

// Comparisons: all-ones lanes where true (vector comparisons yield -1/0).
SSE2SCALAR_INLINE __m128 _mm_cmpeq_ps( __m128 a, __m128 b ) { return sse2scalar_bits<__m128>( a == b ); }
SSE2SCALAR_INLINE __m128 _mm_cmplt_ps( __m128 a, __m128 b ) { return sse2scalar_bits<__m128>( a < b ); }
SSE2SCALAR_INLINE __m128 _mm_cmple_ps( __m128 a, __m128 b ) { return sse2scalar_bits<__m128>( a <= b ); }
SSE2SCALAR_INLINE __m128 _mm_cmpgt_ps( __m128 a, __m128 b ) { return sse2scalar_bits<__m128>( a > b ); }
SSE2SCALAR_INLINE __m128 _mm_cmpge_ps( __m128 a, __m128 b ) { return sse2scalar_bits<__m128>( a >= b ); }
SSE2SCALAR_INLINE int _mm_comigt_ss( __m128 a, __m128 b ) { return a[0] > b[0]; }
SSE2SCALAR_INLINE int _mm_comilt_ss( __m128 a, __m128 b ) { return a[0] < b[0]; }
SSE2SCALAR_INLINE int _mm_movemask_ps( __m128 a )
{
	const __m128i bits = sse2scalar_bits<__m128i>( a );
	return ( bits[0] < 0 ? 1 : 0 ) | ( bits[1] < 0 ? 2 : 0 ) | ( bits[2] < 0 ? 4 : 0 ) |
	       ( bits[3] < 0 ? 8 : 0 );
}

SSE2SCALAR_INLINE __m128 _mm_shuffle_ps_scalar( __m128 a, __m128 b, int imm )
{
	return __m128{ a[imm & 3], a[( imm >> 2 ) & 3], b[( imm >> 4 ) & 3], b[( imm >> 6 ) & 3] };
}
#define _mm_shuffle_ps( a, b, imm ) _mm_shuffle_ps_scalar( ( a ), ( b ), ( imm ) )
SSE2SCALAR_INLINE __m128 _mm_movehl_ps( __m128 a, __m128 b ) { return __m128{ b[2], b[3], a[2], a[3] }; }
SSE2SCALAR_INLINE __m128 _mm_movelh_ps( __m128 a, __m128 b ) { return __m128{ a[0], a[1], b[0], b[1] }; }
SSE2SCALAR_INLINE __m128 _mm_unpacklo_ps( __m128 a, __m128 b ) { return __m128{ a[0], b[0], a[1], b[1] }; }
SSE2SCALAR_INLINE __m128 _mm_unpackhi_ps( __m128 a, __m128 b ) { return __m128{ a[2], b[2], a[3], b[3] }; }
// Transposes the 4x4 matrix whose rows are the four arguments, in place.
#define _MM_TRANSPOSE4_PS( row0, row1, row2, row3 ) \
	do \
	{ \
		const __m128 sse2scalar_t0 = ( row0 ), sse2scalar_t1 = ( row1 ), \
					 sse2scalar_t2 = ( row2 ), sse2scalar_t3 = ( row3 ); \
		( row0 ) = __m128{ sse2scalar_t0[0], sse2scalar_t1[0], sse2scalar_t2[0], sse2scalar_t3[0] }; \
		( row1 ) = __m128{ sse2scalar_t0[1], sse2scalar_t1[1], sse2scalar_t2[1], sse2scalar_t3[1] }; \
		( row2 ) = __m128{ sse2scalar_t0[2], sse2scalar_t1[2], sse2scalar_t2[2], sse2scalar_t3[2] }; \
		( row3 ) = __m128{ sse2scalar_t0[3], sse2scalar_t1[3], sse2scalar_t2[3], sse2scalar_t3[3] }; \
	} while ( 0 )

SSE2SCALAR_INLINE float sse2scalar_round( float x, int mode )
{
	switch ( mode & 3 )
	{
	case _MM_FROUND_TO_NEG_INF: return floorf( x );
	case _MM_FROUND_TO_POS_INF: return ceilf( x );
	case _MM_FROUND_TO_ZERO: return truncf( x );
	default: return rintf( x );
	}
}
SSE2SCALAR_INLINE __m128 _mm_round_ps( __m128 a, int mode )
{
	return __m128{ sse2scalar_round( a[0], mode ), sse2scalar_round( a[1], mode ),
		sse2scalar_round( a[2], mode ), sse2scalar_round( a[3], mode ) };
}
SSE2SCALAR_INLINE __m128 _mm_floor_ps( __m128 a ) { return _mm_round_ps( a, _MM_FROUND_TO_NEG_INF ); }
SSE2SCALAR_INLINE __m128 _mm_ceil_ps( __m128 a ) { return _mm_round_ps( a, _MM_FROUND_TO_POS_INF ); }

// Conversions. cvtt truncates; cvtss rounds in the current mode (nearest).
SSE2SCALAR_INLINE __m128i _mm_cvttps_epi32( __m128 a )
{
	return __m128i{ (int32_t)a[0], (int32_t)a[1], (int32_t)a[2], (int32_t)a[3] };
}
SSE2SCALAR_INLINE __m128 _mm_cvtepi32_ps( __m128i a )
{
	return __m128{ (float)a[0], (float)a[1], (float)a[2], (float)a[3] };
}
SSE2SCALAR_INLINE __m64 _mm_cvttps_pi32( __m128 a ) { return __m64{ (int32_t)a[0], (int32_t)a[1] }; }
SSE2SCALAR_INLINE __m128 _mm_cvtpi32x2_ps( __m64 a, __m64 b )
{
	return __m128{ (float)a[0], (float)a[1], (float)b[0], (float)b[1] };
}
SSE2SCALAR_INLINE int _mm_cvtss_si32( __m128 a ) { return (int)lrintf( a[0] ); }
SSE2SCALAR_INLINE long long _mm_cvtss_si64( __m128 a ) { return llrintf( a[0] ); }
SSE2SCALAR_INLINE __m128 _mm_cvt_si2ss( __m128 a, int b ) { a[0] = (float)b; return a; }

// --- __m128i: four int32 lanes (the engine uses the epi32 forms only) --------

SSE2SCALAR_INLINE __m128i _mm_setzero_si128() { return __m128i{ 0, 0, 0, 0 }; }
SSE2SCALAR_INLINE __m128i _mm_set1_epi32( int a ) { return __m128i{ a, a, a, a }; }
SSE2SCALAR_INLINE __m128i _mm_add_epi32( __m128i a, __m128i b ) { return a + b; }
SSE2SCALAR_INLINE __m128i _mm_sub_epi32( __m128i a, __m128i b ) { return a - b; }
SSE2SCALAR_INLINE __m128i _mm_and_si128( __m128i a, __m128i b ) { return a & b; }
SSE2SCALAR_INLINE __m128i _mm_andnot_si128( __m128i a, __m128i b ) { return ~a & b; }
SSE2SCALAR_INLINE __m128i _mm_cmpeq_epi32( __m128i a, __m128i b ) { return a == b; }
SSE2SCALAR_INLINE __m128i _mm_slli_epi32( __m128i a, int count )
{
	if ( count > 31 )
		return _mm_setzero_si128();
	return sse2scalar_bits<__m128i>( sse2scalar_bits<__m128i>( a ) << count );
}
SSE2SCALAR_INLINE __m128 _mm_castsi128_ps( __m128i a ) { return sse2scalar_bits<__m128>( a ); }

// --- __m64: two int32 lanes (MMX) ---------------------------------------------

SSE2SCALAR_INLINE __m64 _mm_setzero_si64() { return __m64{ 0, 0 }; }
SSE2SCALAR_INLINE __m64 _mm_add_pi32( __m64 a, __m64 b ) { return a + b; }
SSE2SCALAR_INLINE __m64 _mm_sub_pi32( __m64 a, __m64 b ) { return a - b; }
SSE2SCALAR_INLINE __m64 _mm_and_si64( __m64 a, __m64 b ) { return a & b; }
SSE2SCALAR_INLINE __m64 _mm_andnot_si64( __m64 a, __m64 b ) { return ~a & b; }
SSE2SCALAR_INLINE __m64 _mm_cmpeq_pi32( __m64 a, __m64 b ) { return a == b; }
SSE2SCALAR_INLINE __m64 _mm_slli_pi32( __m64 a, int count )
{
	if ( count > 31 )
		return _mm_setzero_si64();
	return a << count;
}
SSE2SCALAR_INLINE void _mm_empty() {}

// --- __m128d: two doubles -----------------------------------------------------

SSE2SCALAR_INLINE __m128d _mm_set1_pd( double a ) { return __m128d{ a, a }; }
SSE2SCALAR_INLINE __m128d _mm_load_pd( const double *p ) { return *(const __m128d *)p; }
SSE2SCALAR_INLINE void _mm_store_pd( double *p, __m128d a ) { *(__m128d *)p = a; }
SSE2SCALAR_INLINE void _mm_store_sd( double *p, __m128d a ) { *p = a[0]; }
SSE2SCALAR_INLINE __m128d _mm_add_pd( __m128d a, __m128d b ) { return a + b; }
SSE2SCALAR_INLINE __m128d _mm_mul_pd( __m128d a, __m128d b ) { return a * b; }
SSE2SCALAR_INLINE __m128d _mm_add_sd( __m128d a, __m128d b ) { a[0] += b[0]; return a; }
SSE2SCALAR_INLINE __m128d _mm_unpackhi_pd( __m128d a, __m128d b ) { return __m128d{ a[1], b[1] }; }
SSE2SCALAR_INLINE __m128d _mm_shuffle_pd_scalar( __m128d a, __m128d b, int imm )
{
	return __m128d{ a[imm & 1], b[( imm >> 1 ) & 1] };
}
#define _mm_shuffle_pd( a, b, imm ) _mm_shuffle_pd_scalar( ( a ), ( b ), ( imm ) )

// --- hints ----------------------------------------------------------------------

#define _mm_prefetch( p, hint ) ( (void)( p ), (void)( hint ) )
SSE2SCALAR_INLINE void _mm_pause() {}

#endif // SSE2SCALAR_H
