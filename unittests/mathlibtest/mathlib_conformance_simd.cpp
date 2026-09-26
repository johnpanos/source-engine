//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: mathlib.core SIMD section: fltx4 operations, FourVectors and the
//          out-of-line SIMD helpers (ssemath.h, powsse.cpp, randsse.cpp,
//          sseconst.cpp). On arm64 these compile through sse2neon.
//
// Contracts:
//   exact      lane arithmetic, comparisons, masks, shuffles, transposes,
//              floor/frac/mod2, integer conversions: bitwise equal to the
//              scalar operation on each lane (MaddSIMD/MsubSIMD may fuse;
//              DivSIMD is 3 ulps, since gcc -ffast-math divides by rcpps)
//   estimate   ReciprocalEstSIMD and ReciprocalSqrtEstSIMD: relative error
//              at most 1.5 * 2^-12, the x86 rcpps/rsqrtps bound the callers
//              were tuned against
//   refined    ReciprocalSIMD, ReciprocalSqrtSIMD and the FourVectors
//              normalize built on them: one Newton step from the estimate,
//              so relative error at most 1e-6
//   libm       SinSIMD, SinCosSIMD, ArcSin/ArcCos/ArcTan2SIMD, ExpSIMD: the
//              scalar function's accuracy (4 ulps)
//   approx     Sin01SIMD 0.0011 and SinEst01SIMD 0.057 absolute, PowSIMD the
//              quarter-exponent rounding it documents
//
//=============================================================================//

#include "mathlib_conformance.h"

#include "mathlib/ssemath.h"

#include <algorithm>
#include <cfloat>

namespace mathconf
{
namespace
{

struct Lanes
{
	float f[4];
};

Lanes Get( const fltx4 &v )
{
	Lanes l;
	std::memcpy( l.f, &v, sizeof( l.f ) );
	return l;
}

uint32 Bits( float f )
{
	uint32 u;
	std::memcpy( &u, &f, sizeof( u ) );
	return u;
}

fltx4 Make( float a, float b, float c, float d )
{
	ALIGN16 float v[4] ALIGN16_POST = { a, b, c, d };
	return LoadAlignedSIMD( v );
}

fltx4 RandomX4( Rng &rng, float lo, float hi )
{
	return Make(
	    rng.Float( lo, hi ), rng.Float( lo, hi ), rng.Float( lo, hi ), rng.Float( lo, hi ) );
}

bool SameBits( const fltx4 &a, const fltx4 &b )
{
	return std::memcmp( &a, &b, sizeof( fltx4 ) ) == 0;
}

// Bad providers --------------------------------------------------------------

fltx4 RealRsqrtEst( const fltx4 &a )
{
	return ReciprocalSqrtEstSIMD( a );
}
fltx4 RealRsqrt( const fltx4 &a )
{
	return ReciprocalSqrtSIMD( a );
}
fltx4 RealRcpEst( const fltx4 &a )
{
	return ReciprocalEstSIMD( a );
}
fltx4 RealRcp( const fltx4 &a )
{
	return ReciprocalSIMD( a );
}
fltx4 BadRsqrtEightBit( const fltx4 &a )
{
	// Keep only 8 bits of mantissa: the precision of an unrefined arm64
	// FRSQRTE.
	Lanes l = Get( ReciprocalSqrtSIMD( a ) );
	for ( float &f : l.f )
	{
		uint32 u = Bits( f ) & 0xFFFF8000u;
		std::memcpy( &f, &u, sizeof( u ) );
	}
	return Make( l.f[0], l.f[1], l.f[2], l.f[3] );
}
fltx4 BadRsqrtUnrefined( const fltx4 &a )
{
	return ReciprocalSqrtEstSIMD( a );
}

void BadSinCosSwapped( fltx4 &s, fltx4 &c, const fltx4 &x )
{
	SinCosSIMD( c, s, x );
}
void RealSinCos( fltx4 &s, fltx4 &c, const fltx4 &x )
{
	SinCosSIMD( s, c, x );
}

fltx4 RealFloor( const fltx4 &x )
{
	return FloorSIMD( x );
}
fltx4 BadFloorTruncate( const fltx4 &x )
{
	Lanes l = Get( x );
	return Make( truncf( l.f[0] ), truncf( l.f[1] ), truncf( l.f[2] ), truncf( l.f[3] ) );
}

// Checkers -------------------------------------------------------------------

typedef fltx4 ( *UnaryFn )( const fltx4 & );
typedef void ( *SinCosFn )( fltx4 &, fltx4 &, const fltx4 & );

void CheckRelative( Checks &c, const char *pszName, UnaryFn fn, bool bSqrt, double budget )
{
	Rng rng( g_nSeed ^ 0x5A );
	for ( int i = 0; i < 20000; ++i )
	{
		float scale = std::ldexp( 1.0f, rng.Int( -30, 30 ) );
		fltx4 x = RandomX4( rng, 1.0f, 2.0f ) * ReplicateX4( scale );
		if ( !bSqrt && ( i & 1 ) )
			x = NegSIMD( x );
		Lanes in = Get( x ), out = Get( fn( x ) );
		for ( int k = 0; k < 4; ++k )
		{
			double want = bSqrt ? 1.0 / std::sqrt( (double)in.f[k] ) : 1.0 / in.f[k];
			c.Sample( pszName, std::fabs( out.f[k] / want - 1.0 ), budget,
			    Fmt( "x %.9g got %.9g", in.f[k], out.f[k] ).c_str() );
		}
	}
}

void CheckSinCos( Checks &c, SinCosFn fn )
{
	Rng rng( g_nSeed ^ 0x5B );
	for ( int i = 0; i < 20000; ++i )
	{
		fltx4 x = RandomX4( rng, -50.0f, 50.0f ), s, co;
		fn( s, co, x );
		Lanes in = Get( x ), ls = Get( s ), lc = Get( co );
		for ( int k = 0; k < 4; ++k )
		{
			c.Sample(
			    "simd.sincos.sin", std::fabs( ls.f[k] - std::sin( (double)in.f[k] ) ), 4.8e-7, "" );
			c.Sample(
			    "simd.sincos.cos", std::fabs( lc.f[k] - std::cos( (double)in.f[k] ) ), 4.8e-7, "" );
		}
	}
}

void CheckFloor( Checks &c, UnaryFn fn )
{
	Rng rng( g_nSeed ^ 0x5C );
	for ( int i = 0; i < 20000; ++i )
	{
		fltx4 x = ( i < 16 ) ? Make( -1.5f + i, -0.5f - i, i * 0.25f, -i * 0.25f )
		                     : RandomX4( rng, -8e6f, 8e6f );
		if ( i & 1 )
			x = RandomX4( rng, -4.0f, 4.0f );
		Lanes in = Get( x ), out = Get( fn( x ) );
		for ( int k = 0; k < 4; ++k )
			c.Check( out.f[k] == std::floor( in.f[k] ), "simd.floor", "x %.9g got %.9g", in.f[k],
			    out.f[k] );
	}
}

void RunLaneOps( Checks &c )
{
	Rng rng( g_nSeed ^ 0x50 );
	for ( int i = 0; i < 20000; ++i )
	{
		fltx4 a = RandomX4( rng, -1e4f, 1e4f ), b = RandomX4( rng, -1e4f, 1e4f ),
		      d = RandomX4( rng, -1e4f, 1e4f );
		if ( i % 8 == 0 )
			b = a; // equal lanes exercise the comparisons
		Lanes la = Get( a ), lb = Get( b ), ld = Get( d );
		Lanes add = Get( AddSIMD( a, b ) ), sub = Get( SubSIMD( a, b ) ),
		      mul = Get( MulSIMD( a, b ) ), div = Get( DivSIMD( a, b ) );
		Lanes mn = Get( MinSIMD( a, b ) ), mx = Get( MaxSIMD( a, b ) ),
		      madd = Get( MaddSIMD( a, b, d ) ), msub = Get( MsubSIMD( a, b, d ) );
		Lanes gt = Get( CmpGtSIMD( a, b ) ), ge = Get( CmpGeSIMD( a, b ) ),
		      lt = Get( CmpLtSIMD( a, b ) ), le = Get( CmpLeSIMD( a, b ) ),
		      eq = Get( CmpEqSIMD( a, b ) );
		Lanes sel = Get( MaskedAssign( CmpGtSIMD( a, b ), a, d ) ), neg = Get( NegSIMD( a ) ),
		      ab = Get( fabs( a ) );
		Lanes sq = Get( SqrtSIMD( fabs( a ) ) );
		for ( int k = 0; k < 4; ++k )
		{
			float x = la.f[k], y = lb.f[k], z = ld.f[k];
			bool exact = add.f[k] == x + y && sub.f[k] == x - y && mul.f[k] == x * y &&
			             mn.f[k] == std::min( x, y ) && mx.f[k] == std::max( x, y );
			c.Check( exact, "simd.lane.arithmetic", "%g %g", x, y );
			// gcc -ffast-math on x86 divides vectors by rcpps and one Newton step
			// (implicit -mrecip), so the product contract is 3 ulps, not IEEE.
			c.Sample(
			    "simd.lane.div", std::fabs( div.f[k] / ( (double)x / y ) - 1.0 ), 3.6e-7, "" );
			double fma = (double)x * y + z,
			       tol = 2 * FLT_EPSILON * ( std::fabs( (double)x * y ) + std::fabs( z ) );
			c.Check( std::fabs( madd.f[k] - fma ) <= tol &&
			             std::fabs( msub.f[k] - ( z - (double)x * y ) ) <= tol,
			    "simd.lane.madd-msub", "%g %g %g", x, y, z );
			bool masks = Bits( gt.f[k] ) == ( x > y ? ~0u : 0u ) &&
			             Bits( ge.f[k] ) == ( x >= y ? ~0u : 0u ) &&
			             Bits( lt.f[k] ) == ( x < y ? ~0u : 0u ) &&
			             Bits( le.f[k] ) == ( x <= y ? ~0u : 0u ) &&
			             Bits( eq.f[k] ) == ( x == y ? ~0u : 0u );
			c.Check( masks, "simd.lane.compare-masks", "%g %g", x, y );
			c.Check( sel.f[k] == ( x > y ? x : z ) && neg.f[k] == -x && ab.f[k] == std::fabs( x ),
			    "simd.lane.select-negate-abs" );
			// Under -ffast-math the compiler may evaluate the scalar reference
			// by estimate and refinement, so the contract is 2 ulps, not bits.
			double wantSqrt = std::sqrt( (double)std::fabs( x ) );
			c.Sample( "simd.lane.sqrt",
			    wantSqrt ? std::fabs( sq.f[k] / wantSqrt - 1.0 ) : std::fabs( sq.f[k] ), 2.4e-7,
			    "" );
		}
		int sign = TestSignSIMD( a );
		int wantSign =
		    ( la.f[0] < 0 ) | ( la.f[1] < 0 ) << 1 | ( la.f[2] < 0 ) << 2 | ( la.f[3] < 0 ) << 3;
		c.Check( sign == wantSign && IsAnyNegative( a ) == ( wantSign != 0 ), "simd.test-sign" );
		bool allGt =
		    la.f[0] > lb.f[0] && la.f[1] > lb.f[1] && la.f[2] > lb.f[2] && la.f[3] > lb.f[3];
		c.Check( IsAllGreaterThan( a, b ) == allGt && IsAllEqual( a, a ) &&
		             ( IsAllEqual( a, b ) == ( i % 8 == 0 ) ),
		    "simd.is-all" );

		// Bitwise logic.
		Lanes andv = Get( AndSIMD( a, b ) ), orv = Get( OrSIMD( a, b ) ),
		      xorv = Get( XorSIMD( a, b ) ), andn = Get( AndNotSIMD( a, b ) );
		bool bits = true;
		for ( int k = 0; k < 4; ++k )
		{
			uint32 x = Bits( la.f[k] ), y = Bits( lb.f[k] );
			bits = bits && Bits( andv.f[k] ) == ( x & y ) && Bits( orv.f[k] ) == ( x | y ) &&
			       Bits( xorv.f[k] ) == ( x ^ y ) && Bits( andn.f[k] ) == ( ~x & y );
		}
		c.Check( bits, "simd.bitwise" );

		// Shuffles.
		fltx4 t0 = a, t1 = b, t2 = d, t3 = AddSIMD( a, d );
		Lanes l3 = Get( t3 );
		TransposeSIMD( t0, t1, t2, t3 );
		Lanes r[4] = { Get( t0 ), Get( t1 ), Get( t2 ), Get( t3 ) };
		const Lanes *src[4] = { &la, &lb, &ld, &l3 };
		bool tr = true;
		for ( int row = 0; row < 4; ++row )
			for ( int col = 0; col < 4; ++col )
				tr = tr && r[row].f[col] == src[col]->f[row];
		c.Check( tr, "simd.transpose" );
		Lanes sx = Get( SplatXSIMD( a ) ), sy = Get( SplatYSIMD( a ) ), sz = Get( SplatZSIMD( a ) ),
		      sw = Get( SplatWSIMD( a ) );
		Lanes rl = Get( RotateLeft( a ) ), rl2 = Get( RotateLeft2( a ) ),
		      rr = Get( RotateRight( a ) ), rr2 = Get( RotateRight2( a ) );
		bool sh = true;
		for ( int k = 0; k < 4; ++k )
		{
			sh = sh && sx.f[k] == la.f[0] && sy.f[k] == la.f[1] && sz.f[k] == la.f[2] &&
			     sw.f[k] == la.f[3];
			sh = sh && rl.f[k] == la.f[( k + 1 ) & 3] && rl2.f[k] == la.f[( k + 2 ) & 3] &&
			     rr.f[k] == la.f[( k + 3 ) & 3] && rr2.f[k] == la.f[( k + 2 ) & 3];
		}
		c.Check( sh, "simd.splat-rotate" );
		Lanes setx = Get( SetXSIMD( a, b ) ), setw = Get( SetWSIMD( a, b ) ),
		      setc = Get( SetComponentSIMD( a, 2, 7.0f ) ), rep = Get( ReplicateX4( la.f[1] ) );
		c.Check( setx.f[0] == lb.f[0] && setx.f[1] == la.f[1] && setw.f[3] == lb.f[3] &&
		             setw.f[0] == la.f[0] && setc.f[2] == 7.0f && setc.f[3] == la.f[3] &&
		             rep.f[0] == la.f[1] && rep.f[3] == la.f[1],
		    "simd.set-replicate" );
		Lanes lo = Get( FindLowestSIMD3( a ) ), hi = Get( FindHighestSIMD3( a ) );
		c.Check( lo.f[0] == std::min( la.f[0], std::min( la.f[1], la.f[2] ) ) &&
		             hi.f[3] == std::max( la.f[0], std::max( la.f[1], la.f[2] ) ),
		    "simd.find-lowest-highest3" );

		// Dot products replicate to every lane.
		Lanes d3 = Get( Dot3SIMD( a, b ) ), d4 = Get( Dot4SIMD( a, b ) );
		double w3 = (double)la.f[0] * lb.f[0] + (double)la.f[1] * lb.f[1] +
		            (double)la.f[2] * lb.f[2],
		       w4 = w3 + (double)la.f[3] * lb.f[3];
		c.Sample( "simd.dot3", std::fabs( d3.f[0] - w3 ) / 4e8, 3e-7, "" );
		c.Sample( "simd.dot4", std::fabs( d4.f[3] - w4 ) / 4e8, 3e-7, "" );
		c.Check( d3.f[0] == d3.f[3] && d4.f[0] == d4.f[2], "simd.dot.replicated" );

		// Unaligned load/store.
		float buf[9] = {};
		StoreUnalignedSIMD( buf + 1, a );
		StoreUnaligned3SIMD( buf + 5, b );
		fltx4 back = LoadUnalignedSIMD( buf + 1 );
		c.Check( SameBits( back, a ) && buf[5] == lb.f[0] && buf[7] == lb.f[2] && buf[8] == 0.0f,
		    "simd.unaligned-load-store" );
	}
}

void RunIntegerOps( Checks &c )
{
	Rng rng( g_nSeed ^ 0x51 );
	for ( int i = 0; i < 5000; ++i )
	{
		ALIGN16 int32 ints[4] ALIGN16_POST;
		ALIGN16 uint32 uints[4] ALIGN16_POST;
		for ( int k = 0; k < 4; ++k )
		{
			ints[k] = (int32)( rng.Next() & 0xFFFFFF ) - 0x800000;
			uints[k] = (uint32)( rng.Next() & 0xFFFFFF );
		}
		Lanes s = Get( SignedIntConvertToFltSIMD( LoadAlignedIntSIMD( ints ) ) );
		Lanes u = Get( UnsignedIntConvertToFltSIMD( LoadAlignedIntSIMD( uints ) ) );
		bool okS = true, okU = true;
		for ( int k = 0; k < 4; ++k )
		{
			okS = okS && s.f[k] == (float)ints[k];
			okU = okU && u.f[k] == (float)uints[k];
		}
		c.Check( okS, "simd.signed-int-convert" );
		c.Check( okU, "simd.unsigned-int-convert", "%u -> %g", uints[0], u.f[0] );

		fltx4 f = RandomX4( rng, -1e6f, 1e6f );
		intx4 out;
		ConvertStoreAsIntsSIMD( &out, f );
		Lanes lf = Get( f );
		c.Check( out[0] == (int)lf.f[0] && out[1] == (int)lf.f[1] && out[2] == (int)lf.f[2] &&
		             out[3] == (int)lf.f[3],
		    "simd.convert-store-as-ints" );
	}
}

void RunRounding( Checks &c )
{
	CheckFloor( c, RealFloor );
	Rng rng( g_nSeed ^ 0x52 );
	for ( int i = 0; i < 20000; ++i )
	{
		fltx4 x = RandomX4( rng, -1000.0f, 1000.0f );
		Lanes in = Get( x ), ce = Get( CeilSIMD( x ) ), fr = Get( FracSIMD( x ) ),
		      m2 = Get( Mod2SIMD( x ) ), m2p = Get( Mod2SIMDPositiveInput( fabs( x ) ) );
		for ( int k = 0; k < 4; ++k )
		{
			float v = in.f[k];
			c.Check( ce.f[k] == std::ceil( v ), "simd.ceil", "%g got %g", v, ce.f[k] );
			// FracSIMD keeps the sign: v - trunc( v ).
			c.Check( fr.f[k] == v - std::trunc( v ), "simd.frac", "%.9g got %.9g", v, fr.f[k] );
			// Mod2SIMD: v - 2 * trunc( v / 2 ), the sign of v.
			c.Check( m2.f[k] == v - 2.0f * std::trunc( v * 0.5f ), "simd.mod2", "%.9g got %.9g", v,
			    m2.f[k] );
			c.Check( m2p.f[k] == std::fabs( v ) - 2.0f * std::trunc( std::fabs( v ) * 0.5f ),
			    "simd.mod2-positive", "%.9g got %.9g", v, m2p.f[k] );
		}
	}
	ExpectRejected( c, "simd.floor.rejects-truncation",
	    []( Checks &s )
	    {
		    CheckFloor( s, BadFloorTruncate );
	    } );
}

void RunReciprocals( Checks &c )
{
	const double kEst = 1.5 * std::ldexp( 1.0, -12 );
	CheckRelative( c, "simd.reciprocal-sqrt-est", RealRsqrtEst, true, kEst );
	CheckRelative( c, "simd.reciprocal-est", RealRcpEst, false, kEst );
	CheckRelative( c, "simd.reciprocal-sqrt", RealRsqrt, true, 1e-6 );
	CheckRelative( c, "simd.reciprocal", RealRcp, false, 1e-6 );

	// Saturating forms return a large finite value for 0.
	Lanes rs = Get( ReciprocalSaturateSIMD( Four_Zeros ) ),
	      rse = Get( ReciprocalEstSaturateSIMD( Four_Zeros ) ),
	      rqs = Get( ReciprocalSqrtEstSaturateSIMD( Four_Zeros ) );
	c.Check( IsFiniteValue( rs.f[0] ) && rs.f[0] > 1e6f && IsFiniteValue( rse.f[1] ) &&
	             rse.f[1] > 1e6f && IsFiniteValue( rqs.f[2] ) && rqs.f[2] > 1e3f,
	    "simd.reciprocal-saturate.zero", "%g %g %g", rs.f[0], rse.f[1], rqs.f[2] );

	ExpectRejected( c, "simd.reciprocal-sqrt.rejects-eight-bit",
	    []( Checks &s )
	    {
		    CheckRelative( s, "bad", BadRsqrtEightBit, true, 1e-6 );
	    } );
	ExpectRejected( c, "simd.reciprocal-sqrt.rejects-unrefined-estimate",
	    []( Checks &s )
	    {
		    CheckRelative( s, "bad", BadRsqrtUnrefined, true, 1e-6 );
	    } );
}

void RunTranscendentals( Checks &c )
{
	CheckSinCos( c, RealSinCos );
	Rng rng( g_nSeed ^ 0x53 );
	for ( int i = 0; i < 20000; ++i )
	{
		fltx4 x = RandomX4( rng, -50.0f, 50.0f ), u = RandomX4( rng, -1.0f, 1.0f ),
		      y = RandomX4( rng, -10.0f, 10.0f ), e = RandomX4( rng, -20.0f, 20.0f );
		Lanes lx = Get( x ), lu = Get( u ), ly = Get( y ), le = Get( e );
		Lanes sn = Get( SinSIMD( x ) ), as = Get( ArcSinSIMD( u ) ), ac = Get( ArcCosSIMD( u ) ),
		      at = Get( ArcTan2SIMD( y, x ) ), ex = Get( ExpSIMD( e ) );
		Lanes s01 = Get( Sin01SIMD( u ) ), se01 = Get( SinEst01SIMD( u ) );
		for ( int k = 0; k < 4; ++k )
		{
			c.Sample( "simd.sin", std::fabs( sn.f[k] - std::sin( (double)lx.f[k] ) ), 4.8e-7, "" );
			c.Sample(
			    "simd.arcsin", std::fabs( as.f[k] - std::asin( (double)lu.f[k] ) ), 4.8e-7, "" );
			c.Sample(
			    "simd.arccos", std::fabs( ac.f[k] - std::acos( (double)lu.f[k] ) ), 9.6e-7, "" );
			c.Sample( "simd.arctan2",
			    std::fabs( at.f[k] - std::atan2( (double)ly.f[k], (double)lx.f[k] ) ), 9.6e-7, "" );
			double p2 = std::exp2( (double)le.f[k] );
			// -ffast-math may evaluate powf( 2, x ) as exp( x ln 2 ), whose
			// argument rounding grows with |x|.
			c.Sample( "simd.exp2",
			    std::fabs( ex.f[k] - p2 ) / p2 / ( 4.8e-7 + 6e-8 * 0.7 * std::fabs( le.f[k] ) ),
			    1.0, "" );
			double s = std::sin( M_PI * lu.f[k] );
			c.Sample(
			    "simd.sin01", std::fabs( s01.f[k] - s ), 0.0011, Fmt( "x %g", lu.f[k] ).c_str() );
			c.Sample( "simd.sin-est01", std::fabs( se01.f[k] - s ), 0.057,
			    Fmt( "x %g", lu.f[k] ).c_str() );
		}
	}

	// PowSIMD rounds the exponent down to a multiple of 1/4 and uses the
	// estimate reciprocal for negative exponents.
	for ( int i = 0; i < 20000; ++i )
	{
		fltx4 x = RandomX4( rng, 0.01f, 4.0f );
		float expo = rng.Int( -24, 24 ) * 0.25f;
		Lanes lx = Get( x ), p = Get( PowSIMD( x, expo ) );
		for ( int k = 0; k < 4; ++k )
		{
			double want = std::pow( (double)lx.f[k], (double)expo );
			c.Sample( expo < 0 ? "simd.pow.negative" : "simd.pow.positive",
			    std::fabs( p.f[k] / want - 1.0 ), expo < 0 ? 4e-4 : 4e-6,
			    Fmt( "x %g e %g", lx.f[k], expo ).c_str() );
		}
	}

	ExpectRejected( c, "simd.sincos.rejects-swapped-outputs",
	    []( Checks &s )
	    {
		    CheckSinCos( s, BadSinCosSwapped );
	    } );
}

void RunFourVectors( Checks &c )
{
	Rng rng( g_nSeed ^ 0x54 );
	for ( int i = 0; i < 5000; ++i )
	{
		Vector v[4], w[4];
		for ( int k = 0; k < 4; ++k )
		{
			v[k] = rng.Vec( -1000, 1000 );
			w[k] = rng.Vec( -1000, 1000 );
		}
		FourVectors a, b;
		a.LoadAndSwizzle( v[0], v[1], v[2], v[3] );
		b.LoadAndSwizzle( w[0], w[1], w[2], w[3] );
		bool swz = true;
		for ( int k = 0; k < 4; ++k )
			swz = swz && a.Vec( k ) == v[k] && a.X( k ) == v[k].x && a.Z( k ) == v[k].z;
		c.Check( swz, "simd.four-vectors.swizzle" );

		Lanes dot = Get( a * b );
		FourVectors cr = a ^ b;
		FourVectors n = a;
		n.VectorNormalize();
		FourVectors nf = a;
		nf.VectorNormalizeFast();
		Lanes len = Get( a.length() );
		matrix3x4_t m;
		AngleMatrix( rng.Angles( 180.0f ), rng.Vec( -1000, 1000 ), m );
		FourVectors t = a, r = a;
		t.TransformBy( m );
		r.RotateBy( m );
		for ( int k = 0; k < 4; ++k )
		{
			DVec dv = DV( v[k] ), dw = DV( w[k] );
			double mag = std::sqrt( ( dv.x * dv.x + dv.y * dv.y + dv.z * dv.z ) *
			                        ( dw.x * dw.x + dw.y * dw.y + dw.z * dw.z ) );
			c.Sample( "simd.four-vectors.dot",
			    std::fabs( dot.f[k] - ( dv.x * dw.x + dv.y * dw.y + dv.z * dw.z ) ) / mag, 4e-7,
			    "" );
			DVec dc = {
			    dv.y * dw.z - dv.z * dw.y, dv.z * dw.x - dv.x * dw.z, dv.x * dw.y - dv.y * dw.x };
			c.Sample( "simd.four-vectors.cross", DMaxDiff( dc, cr.Vec( k ) ) / mag, 4e-7, "" );
			double l = std::sqrt( dv.x * dv.x + dv.y * dv.y + dv.z * dv.z );
			c.Sample( "simd.four-vectors.length", std::fabs( len.f[k] - l ) / l, 2e-7, "" );
			DVec un = { dv.x / l, dv.y / l, dv.z / l };
			c.Sample( "simd.four-vectors.normalize", DMaxDiff( un, n.Vec( k ) ), 1e-6, "" );
			c.Sample( "simd.four-vectors.normalize-fast", DMaxDiff( un, nf.Vec( k ) ),
			    1.5 * std::ldexp( 1.0, -12 ), "" );
			DVec tw = DMatApply( DMatFrom( m ), dv );
			c.Sample(
			    "simd.four-vectors.transform-by", DMaxDiff( tw, t.Vec( k ) ) / 3000.0, 4e-7, "" );
			DMat rot = DMatFrom( m );
			rot.m[0][3] = rot.m[1][3] = rot.m[2][3] = 0;
			c.Sample( "simd.four-vectors.rotate-by",
			    DMaxDiff( DMatApply( rot, dv ), r.Vec( k ) ) / 3000.0, 4e-7, "" );
		}
	}
}

void RunRandom( Checks &c )
{
	SeedRandSIMD( 12345 );
	float first[64];
	bool range = true;
	double sum = 0;
	for ( int i = 0; i < 16; ++i )
	{
		Lanes l = Get( RandSIMD() );
		for ( int k = 0; k < 4; ++k )
			first[i * 4 + k] = l.f[k];
	}
	for ( int i = 0; i < 25000; ++i )
	{
		Lanes l = Get( RandSIMD() );
		for ( float f : l.f )
		{
			range = range && f >= 0.0f && f < 1.0f;
			sum += f;
		}
	}
	c.Check( range, "simd.rand.range" );
	c.Check( std::fabs( sum / 100000.0 - 0.5 ) < 0.01, "simd.rand.mean", "%g", sum / 100000.0 );
	SeedRandSIMD( 12345 );
	bool same = true;
	for ( int i = 0; i < 16; ++i )
	{
		Lanes l = Get( RandSIMD() );
		for ( int k = 0; k < 4; ++k )
			same = same && first[i * 4 + k] == l.f[k];
	}
	c.Check( same, "simd.rand.reseed-repeats" );
}

} // namespace

void RunSimdSection( Checks &c )
{
	RunLaneOps( c );
	RunIntegerOps( c );
	RunRounding( c );
	RunReciprocals( c );
	RunTranscendentals( c );
	RunFourVectors( c );
	RunRandom( c );
}

} // namespace mathconf
