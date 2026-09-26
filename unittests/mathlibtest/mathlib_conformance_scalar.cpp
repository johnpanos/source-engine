//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: mathlib.core scalar section: vector arithmetic, normalization,
//          the MathLib_Init function pointers, angle helpers and the
//          float-to-integer conversions (mathlib.h, mathlib_base.cpp).
//
// The float-to-integer helpers have exact contracts: RoundFloatToInt and
// RoundFloatToUnsignedLong return the nearest integer (a tie may go to
// either neighbour), Floor2Int/Ceil2Int are floor/ceil, Float2Int
// truncates. They are checked on every profile with the same inputs,
// including negative values, so a platform branch that disagrees fails.
//
//=============================================================================//

#include "mathlib_conformance.h"

#include <algorithm>
#include <cfloat>
#include <climits>

namespace mathconf
{
namespace
{

// Bad providers --------------------------------------------------------------

int BadRoundAddHalf( float f ) { return (int)( f + 0.5f ); } // wrong below zero
int BadFloorTruncate( float f ) { return (int)f; }
float FASTCALL BadNormalizeNoLength( Vector &v )
{
	float r = VectorNormalize( v );
	return r * 0.5f;
}
float BadAngleNormalizeWrap( float a ) { return fmodf( a, 360.0f ); }

// Checkers -------------------------------------------------------------------

typedef int ( *IntFn )( float );
typedef float( FASTCALL *NormalizeFn )( Vector & );
typedef float ( *AngleFn )( float );

bool IsTie( float f )
{
	return std::fabs( f - std::floor( f ) - 0.5 ) == 0.0;
}

void CheckRoundToNearest( Checks &c, const char *pszName, IntFn fn )
{
	Rng rng( g_nSeed ^ 0x12 );
	for ( int i = 0; i < 20000; ++i )
	{
		float f;
		if ( i < 64 )
			f = ( i - 32 ) * 0.5f; // ties and integers
		else if ( i & 1 )
			f = rng.Float( -3.0f, 3.0f );
		else
			f = rng.Float( -1e6f, 1e6f );
		int got = fn( f );
		double lo = std::floor( (double)f ), hi = std::ceil( (double)f );
		bool ok = IsTie( f ) ? ( got == (int)lo || got == (int)hi ) : got == (int)std::nearbyint( (double)f );
		c.Check( ok, pszName, "f %.9g got %d", f, got );
	}
}

void CheckFloor( Checks &c, IntFn fn )
{
	Rng rng( g_nSeed ^ 0x13 );
	for ( int i = 0; i < 20000; ++i )
	{
		float f = ( i < 64 ) ? ( i - 32 ) * 0.25f : rng.Float( -1e6f, 1e6f );
		c.Check( fn( f ) == (int)std::floor( (double)f ), "scalar.floor2int", "f %.9g got %d", f, fn( f ) );
	}
}

void CheckNormalize( Checks &c, NormalizeFn fn )
{
	Rng rng( g_nSeed ^ 0x14 );
	for ( int i = 0; i < 20000; ++i )
	{
		Vector v = rng.Vec( -1e4f, 1e4f );
		if ( i & 1 )
			v *= 1e-3f;
		Vector in = v;
		double len = std::sqrt( (double)in.x * in.x + (double)in.y * in.y + (double)in.z * in.z );
		float r = fn( v );
		c.Sample( "scalar.vector-normalize.length", std::fabs( r - len ) / len, 3e-7, "" );
		// The contract scales by 1 / ( |v| + FLT_EPSILON ), so short vectors
		// come back slightly shorter than unit.
		double k = 1.0 / ( len + FLT_EPSILON );
		double e = std::fmax( std::fabs( v.x - in.x * k ), std::fmax( std::fabs( v.y - in.y * k ), std::fabs( v.z - in.z * k ) ) );
		// sqrt, reciprocal and scale: five ulps of a unit component.
		c.Sample( "scalar.vector-normalize.direction", e, 6e-7, "" );
	}
	Vector zero( 0, 0, 0 );
	float r = fn( zero );
	c.Check( r == 0.0f && zero.x == 0.0f && zero.y == 0.0f && zero.z == 0.0f, "scalar.vector-normalize.zero", "r %g", r );
}

void CheckAngleNormalize( Checks &c, AngleFn fn )
{
	Rng rng( g_nSeed ^ 0x15 );
	for ( int i = 0; i < 20000; ++i )
	{
		float a = rng.Float( -3000.0f, 3000.0f );
		float n = fn( a );
		double k = ( (double)a - n ) / 360.0;
		c.Check( n >= -180.0f && n <= 180.0f && std::fabs( k - std::nearbyint( k ) ) < 1e-5, "scalar.angle-normalize", "a %.9g got %.9g", a, n );
	}
}

void RunVectors( Checks &c )
{
	Rng rng( g_nSeed ^ 0x16 );
	for ( int i = 0; i < 20000; ++i )
	{
		Vector a = rng.Vec( -1e3f, 1e3f ), b = rng.Vec( -1e3f, 1e3f );
		DVec da = DV( a ), db = DV( b );
		double dot = da.x * db.x + da.y * db.y + da.z * db.z;
		double mag = std::sqrt( ( da.x * da.x + da.y * da.y + da.z * da.z ) * ( db.x * db.x + db.y * db.y + db.z * db.z ) );
		c.Sample( "scalar.dot", std::fabs( DotProduct( a, b ) - dot ) / mag, 4e-7, "" );
		Vector x = CrossProduct( a, b );
		DVec dx = { da.y * db.z - da.z * db.y, da.z * db.x - da.x * db.z, da.x * db.y - da.y * db.x };
		c.Sample( "scalar.cross", DMaxDiff( dx, x ) / mag, 4e-7, "" );
		float in[ 3 ] = { a.x, a.y, a.z }, out[ 3 ];
		CrossProduct( in, &b.x, out );
		c.Check( out[ 0 ] == x.x && out[ 1 ] == x.y && out[ 2 ] == x.z, "scalar.cross.array-form" );
		double len = std::sqrt( da.x * da.x + da.y * da.y + da.z * da.z );
		c.Sample( "scalar.length", std::fabs( a.Length() - len ) / len, 3e-7, "" );
		c.Sample( "scalar.distance", std::fabs( a.DistTo( b ) - std::sqrt( ( da.x - db.x ) * ( da.x - db.x ) + ( da.y - db.y ) * ( da.y - db.y ) + ( da.z - db.z ) * ( da.z - db.z ) ) ) / ( 1 + len + b.Length() ), 4e-7, "" );
		Vector ma;
		float s = rng.Float( -4, 4 );
		VectorMA( a, s, b, ma );
		c.Sample( "scalar.vector-ma", DMaxDiff( DVec{ da.x + s * db.x, da.y + s * db.y, da.z + s * db.z }, ma ) / ( 1 + len + 4 * mag / len ), 4e-7, "" );
		Vector lerp;
		float t = rng.Float( 0, 1 );
		VectorLerp( a, b, t, lerp );
		c.Sample( "scalar.vector-lerp", DMaxDiff( DVec{ da.x + t * ( db.x - da.x ), da.y + t * ( db.y - da.y ), da.z + t * ( db.z - da.z ) }, lerp ) / 2000.0, 4e-7, "" );
	}

	CheckNormalize( c, VectorNormalize );
	for ( int i = 0; i < 20000; ++i )
	{
		Vector v = rng.Vec( -1e4f, 1e4f ), in = v;
		VectorNormalizeFast( v );
		double len = std::sqrt( (double)in.x * in.x + (double)in.y * in.y + (double)in.z * in.z );
		double e = std::fmax( std::fabs( v.x - in.x / len ), std::fmax( std::fabs( v.y - in.y / len ), std::fabs( v.z - in.z / len ) ) );
		// "Fast" permits the SSE estimate path: 12-bit rsqrt plus one Newton step.
		c.Sample( "scalar.vector-normalize-fast", e, 2e-6, "" );
	}

	// The MathLib_Init function pointers.
	for ( int i = 0; i < 20000; ++i )
	{
		float x = ( i & 1 ) ? rng.Float( 1e-6f, 1.0f ) : rng.Float( 1.0f, 1e8f );
		double s = std::sqrt( (double)x );
		c.Sample( "scalar.pf-sqrt", std::fabs( FastSqrt( x ) - s ) / s, 2e-7, "" );
		c.Sample( "scalar.pf-rsqrt", std::fabs( FastRSqrt( x ) - 1 / s ) * s, 3e-7, "" );
		c.Sample( "scalar.pf-rsqrt-fast", std::fabs( FastRSqrtFast( x ) - 1 / s ) * s, 2e-6, "" );
		Vector v = rng.Vec( -8, 8 );
		double r2 = DotProduct( v, v );
		c.Sample( "scalar.inv-r-squared", std::fabs( InvRSquared( v ) - ( r2 < 1 ? 1.0 : 1 / r2 ) ) * std::max( r2, 1.0 ), 2e-6, "" );
	}
	c.Check( MathLib_SSEEnabled() == GetCPUInformation()->m_bSSE, "scalar.mathlib-init.sse-selection" );

	ExpectRejected( c, "scalar.vector-normalize.rejects-wrong-length", []( Checks &s ) { CheckNormalize( s, BadNormalizeNoLength ); } );
}

void RunAngleHelpers( Checks &c )
{
	CheckAngleNormalize( c, AngleNormalize );
	Rng rng( g_nSeed ^ 0x17 );
	for ( int i = 0; i < 20000; ++i )
	{
		float a = rng.Float( -3000, 3000 ), b = rng.Float( -3000, 3000 );
		float p = AngleNormalizePositive( a );
		double k = ( (double)a - p ) / 360.0;
		c.Check( p >= 0.0f && p <= 360.0f && std::fabs( k - std::nearbyint( k ) ) < 1e-5, "scalar.angle-normalize-positive", "a %g got %g", a, p );

		float m = anglemod( a );
		double km = ( (double)a - m ) / 360.0;
		// anglemod quantizes to 1/65536 of a turn.
		c.Check( m >= 0.0f && m < 360.0f && std::fabs( km - std::nearbyint( km ) ) < 2e-5, "scalar.anglemod", "a %g got %g", a, m );

		float d = AngleDiff( a, b );
		double kd = ( (double)a - b - d ) / 360.0;
		c.Check( d >= -180.0f && d <= 180.0f && std::fabs( kd - std::nearbyint( kd ) ) < 1e-5, "scalar.angle-diff", "%g - %g got %g", a, b, d );

		float na = AngleNormalize( a ), nb = AngleNormalize( b );
		float ad = AngleDistance( na, nb );
		double kad = ( (double)na - nb - ad ) / 360.0;
		c.Check( ad >= -180.0f && ad <= 180.0f && std::fabs( kad - std::nearbyint( kad ) ) < 1e-5, "scalar.angle-distance", "%g %g got %g", na, nb, ad );

		float speed = rng.Float( 0, 50 );
		float ap = Approach( a, b, speed );
		double want = ( a - b > speed ) ? b + speed : ( a - b < -speed ) ? b - speed : a;
		c.Check( ap == (float)want, "scalar.approach", "target %g value %g speed %g got %g", a, b, speed, ap );

		float aa = ApproachAngle( a, b, speed );
		float remaining = AngleDiff( a, aa ), before = AngleDiff( a, b );
		bool reached = std::fabs( before ) <= speed + 1e-2f;
		c.Check( reached ? std::fabs( remaining ) < 2e-2f : std::fabs( std::fabs( before ) - std::fabs( remaining ) - speed ) < 2e-2f,
			"scalar.approach-angle", "target %g value %g speed %g got %g", a, b, speed, aa );
	}

	ExpectRejected( c, "scalar.angle-normalize.rejects-plain-fmod", []( Checks &s ) { CheckAngleNormalize( s, BadAngleNormalizeWrap ); } );
}

int RoundInt( float f ) { return RoundFloatToInt( f ); }
int FloorInt( float f ) { return Floor2Int( f ); }
int RoundUnsigned( float f ) { return (int)RoundFloatToUnsignedLong( f ); }

void RunIntegerConversions( Checks &c )
{
	CheckRoundToNearest( c, "scalar.round-float-to-int", RoundInt );
	CheckFloor( c, FloorInt );

	Rng rng( g_nSeed ^ 0x18 );
	for ( int i = 0; i < 20000; ++i )
	{
		float f = ( i < 64 ) ? ( i - 32 ) * 0.25f : rng.Float( -1e6f, 1e6f );
		c.Check( Ceil2Int( f ) == (int)std::ceil( (double)f ), "scalar.ceil2int", "f %.9g got %d", f, Ceil2Int( f ) );
		c.Check( Float2Int( f ) == (int)std::trunc( (double)f ), "scalar.float2int", "f %.9g got %d", f, Float2Int( f ) );
		float s = rng.Float( -4194304.0f, 4194303.0f );
		c.Check( FastFloatToSmallInt( s ) == (int)std::trunc( (double)s ) || FastFloatToSmallInt( s ) == (int)std::nearbyint( (double)s ),
			"scalar.fast-float-to-small-int", "f %.9g got %d", s, FastFloatToSmallInt( s ) );
		float col = rng.Float( 0.0f, 1.0f );
		unsigned int ftoc = FastFToC( col );
		double c255 = col * 255.0;
		c.Check( ftoc == (unsigned)std::floor( c255 ) || ftoc == (unsigned)std::nearbyint( c255 ), "scalar.fast-ftoc", "f %.9g got %u", col, ftoc );
		float b = rng.Float( 0.0f, 255.0f );
		unsigned char byte = RoundFloatToByte( b );
		c.Check( IsTie( b ) ? ( byte == (int)std::floor( b ) || byte == (int)std::ceil( b ) ) : byte == (int)std::nearbyint( (double)b ),
			"scalar.round-float-to-byte", "f %.9g got %u", b, byte );
	}

	// RoundFloatToUnsignedLong: nearest, and the whole unsigned long is the
	// value (no bits outside the result).
	for ( int i = 0; i < 20000; ++i )
	{
		float f = ( i < 64 ) ? i * 0.5f : rng.Float( 0.0f, 2.0e9f );
		unsigned long got = RoundFloatToUnsignedLong( f );
		double lo = std::floor( (double)f ), hi = std::ceil( (double)f );
		bool ok = IsTie( f ) ? ( got == (unsigned long)lo || got == (unsigned long)hi ) : got == (unsigned long)std::nearbyint( (double)f );
		c.Check( ok, "scalar.round-float-to-unsigned-long", "f %.9g got %lu (0x%lx)", f, got, got );
	}

	for ( unsigned int x = 1; x < 70000; x = x * 3 / 2 + 1 )
	{
		unsigned int p = SmallestPowerOfTwoGreaterOrEqual( x ), q = LargestPowerOfTwoLessThanOrEqual( x );
		c.Check( ( p & ( p - 1 ) ) == 0 && p >= x && p / 2 < x, "scalar.smallest-pow2-ge", "x %u got %u", x, p );
		c.Check( ( q & ( q - 1 ) ) == 0 && q <= x && q * 2 > x, "scalar.largest-pow2-le", "x %u got %u", x, q );
		c.Check( Q_log2( (int)x ) == (int)std::floor( std::log2( (double)x ) ), "scalar.q-log2", "x %u got %d", x, Q_log2( (int)x ) );
	}
	for ( int i = 0; i < 2000; ++i )
	{
		int a = rng.Int( 1, 100000 ), b = rng.Int( 1, 100000 );
		int g = GreatestCommonDivisor( a, b );
		int ra = a, rb = b;
		while ( rb )
		{
			int t = ra % rb;
			ra = rb;
			rb = t;
		}
		c.Check( g == ra, "scalar.gcd", "%d %d got %d", a, b, g );
		// FloorDivMod takes integral values carried in doubles.
		double numer = rng.Int( -1000000, 1000000 ), denom = rng.Int( 1, 1000 );
		int quo, rem;
		FloorDivMod( numer, denom, &quo, &rem );
		c.Check( quo == (int)std::floor( numer / denom ) && rem == (int)( numer - quo * denom ), "scalar.floor-div-mod", "%g / %g got %d r %d", numer, denom, quo, rem );
	}

	// AlmostEqual counts ulps across zero and rejects NaN.
	c.Check( AlmostEqual( 1.0f, std::nextafter( 1.0f, 2.0f ), 1 ), "scalar.almost-equal.one-ulp" );
	c.Check( !AlmostEqual( 1.0f, 1.0f + 64 * FLT_EPSILON, 10 ), "scalar.almost-equal.far" );
	c.Check( AlmostEqual( -0.0f, 0.0f, 0 ), "scalar.almost-equal.signed-zero" );
	// Non-finite inputs are outside the contract of code built with
	// -ffinite-math-only (every product: -ffast-math), where clang may assume
	// x == x. The strict headless build checks them.
#if !defined( __FINITE_MATH_ONLY__ ) || !__FINITE_MATH_ONLY__
	auto fromBits = []( uint32 u ) {
		float f;
		std::memcpy( &f, &u, sizeof( f ) );
		return f;
	};
	volatile uint32 nanBits = 0x7FC00000u, infBits = 0x7F800000u;
	float qnan = fromBits( nanBits ), inf = fromBits( infBits );
	c.Check( !AlmostEqual( qnan, qnan, 10 ) && !AlmostEqual( qnan, 1.0f, 10 ), "scalar.almost-equal.nan" );
	c.Check( AlmostEqual( inf, inf, 10 ) && !AlmostEqual( FLT_MAX, inf, 10 ), "scalar.almost-equal.infinity" );
#endif

	ExpectRejected( c, "scalar.round-float-to-int.rejects-add-half", []( Checks &s ) { CheckRoundToNearest( s, "bad", BadRoundAddHalf ); } );
	ExpectRejected( c, "scalar.floor2int.rejects-truncation", []( Checks &s ) { CheckFloor( s, BadFloorTruncate ); } );
}

void RunCurves( Checks &c )
{
	Rng rng( g_nSeed ^ 0x19 );
	for ( int i = 0; i < 5000; ++i )
	{
		float x = rng.Float( 0, 1 );
		double ss = 3 * (double)x * x - 2 * (double)x * x * x;
		c.Sample( "scalar.simple-spline", std::fabs( SimpleSpline( x ) - ss ), 4e-7, "" );
		float a = rng.Float( -100, 100 ), b = a + rng.Float( 1, 100 ), cc = rng.Float( -100, 100 ), d = rng.Float( -100, 100 );
		float v = rng.Float( a - 50, b + 50 );
		double rv = cc + ( d - cc ) * ( (double)v - a ) / ( (double)b - a );
		// Float rounding of v - A and B - A is relative to the operands, and
		// the quotient scales it by 1 / ( B - A ).
		double cond = 1 + ( std::fabs( v ) + std::fabs( a ) + std::fabs( b ) ) / ( (double)b - a );
		c.Sample( "scalar.remap-val", std::fabs( RemapVal( v, a, b, cc, d ) - rv ) / ( ( 1 + std::fabs( cc ) + std::fabs( d ) ) * cond ), 1e-6, "" );
		double rvc = cc + ( d - cc ) * std::clamp( ( (double)v - a ) / ( (double)b - a ), 0.0, 1.0 );
		c.Sample( "scalar.remap-val-clamped", std::fabs( RemapValClamped( v, a, b, cc, d ) - rvc ) / ( 1 + std::fabs( cc ) + std::fabs( d ) ), 2e-6, "" );
		float bias = rng.Float( 0.05f, 0.95f );
		// Bias( x, b ) = x^( log b / log 0.5 ), so Bias( 0.5, b ) = b.
		double bs = std::pow( (double)x, std::log( (double)bias ) / std::log( 0.5 ) );
		c.Sample( "scalar.bias", std::fabs( Bias( x, bias ) - bs ), 2e-5, Fmt( "x %g b %g", x, bias ).c_str() );
		float s, co;
		float rad = rng.Float( -100, 100 );
		SinCos( rad, &s, &co );
		c.Sample( "scalar.sincos", std::fmax( std::fabs( s - std::sin( (double)rad ) ), std::fabs( co - std::cos( (double)rad ) ) ), 2e-7, "" );
	}
}

} // namespace

void RunScalarSection( Checks &c )
{
	RunVectors( c );
	RunAngleHelpers( c );
	RunIntegerConversions( c );
	RunCurves( c );
}

} // namespace mathconf
