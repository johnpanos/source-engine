//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: mathlib.core conversion section: the float16 codec (exhaustive),
//          compressed animation quaternions and vectors, sRGB and gamma
//          transfer functions, RGBE lightmap colours, the Halton sequence
//          and the ICE block cipher (compressed_vector.h, color_conversion.cpp,
//          halton.cpp, IceKey.cpp).
//
// float16 is Source's own codec, not IEEE binary16 conversion: encoding
// truncates, maps infinities to +-65504 (the largest finite half) and NaN to
// 0, and decoding maps the exponent-31 codes the same way. Every finite half
// decodes exactly to its IEEE value and re-encodes to itself, which is what a
// hardware decode (F16C, FCVT) must preserve to replace it.
//
//=============================================================================//

#include "mathlib_conformance.h"

#include "mathlib/compressed_vector.h"
#include "mathlib/halton.h"
#include "mathlib/IceKey.H"

#include <algorithm>

namespace mathconf
{
namespace
{

// IEEE binary16 decode in double.
double RefHalf( unsigned short h )
{
	int sign = h >> 15, exp = ( h >> 10 ) & 31, man = h & 1023;
	double v;
	if ( exp == 0 )
		v = man * std::ldexp( 1.0, -24 );
	else
		v = ( 1024 + man ) * std::ldexp( 1.0, exp - 25 );
	return sign ? -v : v;
}

typedef float ( *HalfDecodeFn )( unsigned short );
float RealHalfDecode( unsigned short h )
{
	float16 f;
	std::memcpy( (void *)&f, &h, sizeof( h ) );
	return f.GetFloat();
}
float BadHalfDecodeNoDenormals( unsigned short h )
{
	return ( ( h >> 10 ) & 31 ) == 0 ? 0.0f : RealHalfDecode( h );
}
unsigned short HalfEncode( float x )
{
	float16 f;
	f.SetFloat( x );
	return f.GetBits();
}

void CheckHalfDecode( Checks &c, HalfDecodeFn fn )
{
	for ( unsigned int h = 0; h < 65536; ++h )
	{
		int exp = ( h >> 10 ) & 31, man = h & 1023;
		float got = fn( (unsigned short)h );
		double want;
		if ( exp == 31 )
			want = man ? 0.0 : ( ( h & 0x8000 ) ? -65504.0 : 65504.0 );
		else
			want = RefHalf( (unsigned short)h );
		c.Check( (double)got == want, "conversion.float16.decode", "0x%04x got %.9g want %.9g", h, got, want );
	}
}

void RunFloat16( Checks &c )
{
	CheckHalfDecode( c, RealHalfDecode );

	// Every finite half re-encodes to itself (the codec is a retraction onto
	// the finite halves; -0 keeps its sign).
	for ( unsigned int h = 0; h < 65536; ++h )
	{
		if ( ( ( h >> 10 ) & 31 ) == 31 )
			continue;
		unsigned short back = HalfEncode( RealHalfDecode( (unsigned short)h ) );
		c.Check( back == h, "conversion.float16.round-trip", "0x%04x -> 0x%04x", h, back );
	}

	// Encoding truncates toward zero: the result is the largest-magnitude
	// half not exceeding |x| (normal range), and saturates at 65504.
	Rng rng( g_nSeed ^ 0xF1 );
	for ( int i = 0; i < 20000; ++i )
	{
		float x = ( i & 1 ) ? rng.Float( -70000.0f, 70000.0f ) : rng.Float( -1.0f, 1.0f ) * std::ldexp( 1.0f, rng.Int( -14, 15 ) );
		if ( std::fabs( x ) < std::ldexp( 1.0f, -14 ) )
			continue;
		unsigned short h = HalfEncode( x );
		double v = RefHalf( h ), ax = std::fabs( (double)x );
		bool ok = ( ax >= 65504.0 ) ? std::fabs( v ) == 65504.0
									: ( std::fabs( v ) <= ax && std::fabs( RefHalf( ( h & 0x7fff ) + 1 ) ) > ax && ( v < 0 ) == ( x < 0 ) );
		c.Check( ok, "conversion.float16.encode-truncates", "x %.9g got 0x%04x (%g)", x, h, v );
	}
	c.Check( HalfEncode( INFINITY ) == 0x7bff && HalfEncode( -INFINITY ) == 0xfbff, "conversion.float16.encode-infinity" );
	c.Check( ( HalfEncode( NAN ) & 0x7fff ) == 0, "conversion.float16.encode-nan" );

	for ( int i = 0; i < 5000; ++i )
	{
		Vector v = rng.Vec( -60000, 60000 );
		Vector48 p;
		p = v;
		Vector back = p;
		double e = std::fmax( std::fabs( back.x - v.x ) / ( std::fabs( v.x ) + 1e-3 ), std::fmax( std::fabs( back.y - v.y ) / ( std::fabs( v.y ) + 1e-3 ), std::fabs( back.z - v.z ) / ( std::fabs( v.z ) + 1e-3 ) ) );
		c.Sample( "conversion.vector48.relative", e, std::ldexp( 1.0, -10 ), "" );
	}

	ExpectRejected( c, "conversion.float16.decode.rejects-flushed-denormals", []( Checks &s ) { CheckHalfDecode( s, BadHalfDecodeNoDenormals ); } );
}

void RunCompressedQuaternions( Checks &c )
{
	Rng rng( g_nSeed ^ 0xF2 );
	for ( int i = 0; i < 20000; ++i )
	{
		Quaternion q = rng.UnitQuat();
		Quaternion48 q48;
		q48 = q;
		Quaternion d48 = q48;
		// Quantization: x, y truncate to 1/32768, z to 1/16384; w is rebuilt
		// from the unit norm, so its error grows as |w| -> 0.
		double e48 = std::fmax( std::fabs( d48.x - q.x ), std::fmax( std::fabs( d48.y - q.y ), std::fabs( d48.z - q.z ) ) );
		c.Sample( "conversion.quaternion48.xyz", e48, 1.0 / 16384 + 1e-6, "" );
		c.Check( ( d48.w < 0 ) == ( q.w < 0 ) || q.w == 0.0f, "conversion.quaternion48.w-sign" );
		c.Sample( "conversion.quaternion48.rotation", 1.0 - std::fabs( QuaternionDotProduct( d48, q ) ), 4e-4, "" );

		Quaternion64 q64;
		q64 = q;
		Quaternion d64 = q64;
		double e64 = std::fmax( std::fabs( d64.x - q.x ), std::fmax( std::fabs( d64.y - q.y ), std::fabs( d64.z - q.z ) ) );
		c.Sample( "conversion.quaternion64.xyz", e64, 1.0 / 1048576 + 1e-6, "" );
		c.Sample( "conversion.quaternion64.rotation", 1.0 - std::fabs( QuaternionDotProduct( d64, q ) ), 2e-6, "" );
	}
}

void RunTransferFunctions( Checks &c )
{
	for ( int i = 0; i <= 4096; ++i )
	{
		double x = i / 4096.0;
		double lin = ( x <= 0.04045 ) ? x / 12.92 : std::pow( ( x + 0.055 ) / 1.055, 2.4 );
		c.Sample( "conversion.srgb-to-linear", std::fabs( SrgbGammaToLinear( (float)x ) - lin ), 2e-6, "" );
		double gam = ( x <= 0.0031308 ) ? x * 12.92 : 1.055 * std::pow( x, 1 / 2.4 ) - 0.055;
		c.Sample( "conversion.linear-to-srgb", std::fabs( SrgbLinearToGamma( (float)x ) - gam ), 2e-6, "" );
		c.Sample( "conversion.gamma-to-linear-full-range", std::fabs( GammaToLinearFullRange( (float)x ) - std::pow( x, 2.2 ) ), 2e-6, "" );
		c.Sample( "conversion.linear-to-gamma-full-range", std::fabs( LinearToGammaFullRange( (float)x ) - std::pow( x, 1 / 2.2 ) ), 2e-6, "" );
		// The tabled forms return entry round( x * 255 ) of a 256-entry table
		// of the full-range curve (GammaToLinear saturates from 0.95).
		double idx = x * 255.0;
		if ( std::fabs( idx - std::floor( idx ) - 0.5 ) > 1e-3 )
		{
			double k = std::nearbyint( idx ) / 255.0;
			if ( x < 0.95 )
				c.Sample( "conversion.gamma-to-linear-table", std::fabs( GammaToLinear( (float)x ) - std::pow( k, 2.2 ) ), 2e-6, "" );
			c.Sample( "conversion.linear-to-gamma-table", std::fabs( LinearToGamma( (float)x ) - std::pow( k, 1 / 2.2 ) ), 2e-6, "" );
		}
	}
	c.Check( SrgbGammaToLinear( -1.0f ) == 0.0f && SrgbGammaToLinear( 2.0f ) == 1.0f, "conversion.srgb-to-linear.clamps" );

	// RGBE lightmap colours: the largest channel keeps 7 bits of mantissa.
	Rng rng( g_nSeed ^ 0xF3 );
	for ( int i = 0; i < 20000; ++i )
	{
		float scale = std::ldexp( 1.0f, rng.Int( -20, 20 ) );
		Vector v( rng.Float( 0, 1 ) * scale, rng.Float( 0, 1 ) * scale, rng.Float( 0, 1 ) * scale );
		ColorRGBExp32 rgbe;
		VectorToColorRGBExp32( v, rgbe );
		Vector back;
		ColorRGBExp32ToVector( rgbe, back );
		float mx = std::max( v.x, std::max( v.y, v.z ) );
		double e = std::fmax( std::fabs( back.x - v.x ), std::fmax( std::fabs( back.y - v.y ), std::fabs( back.z - v.z ) ) );
		c.Sample( "conversion.rgbe.round-trip", e / mx, 1.0 / 128, Fmt( "(%g %g %g)", v.x, v.y, v.z ).c_str() );
	}
}

void RunHaltonAndIce( Checks &c )
{
	const int bases[] = { 2, 3, 5, 7 };
	for ( int b : bases )
	{
		// GetElement( i ) is the radical inverse of i in base b. NextValue
		// walks the sequence from element 2, as it always has (the sampler
		// callers' observable sequence).
		HaltonSequenceGenerator_t gen( b ), walker( b );
		for ( int i = 0; i < 4096; ++i )
		{
			auto radicalInverse = [ b ]( int n ) {
				double inv = 0, f = 1.0 / b;
				for ( ; n > 0; n /= b, f /= b )
					inv += ( n % b ) * f;
				return inv;
			};
			c.Sample( "conversion.halton.get-element", std::fabs( gen.GetElement( i ) - radicalInverse( i ) ), 2e-7, Fmt( "base %d i %d", b, i ).c_str() );
			c.Sample( "conversion.halton.next-value", std::fabs( walker.NextValue() - radicalInverse( i + 2 ) ), 2e-7, Fmt( "base %d i %d", b, i ).c_str() );
		}
	}

	// ICE: decrypt inverts encrypt at every level; a one-bit key change
	// changes the block.
	Rng rng( g_nSeed ^ 0xF4 );
	for ( int level = 0; level <= 2; ++level )
	{
		IceKey ice( level ), other( level );
		unsigned char key[ 32 ];
		for ( unsigned char &k : key )
			k = (unsigned char)rng.Next();
		ice.set( key );
		key[ 0 ] ^= 1;
		other.set( key );
		for ( int i = 0; i < 500; ++i )
		{
			unsigned char plain[ 8 ], enc[ 8 ], dec[ 8 ], enc2[ 8 ];
			for ( unsigned char &p : plain )
				p = (unsigned char)rng.Next();
			ice.encrypt( plain, enc );
			ice.decrypt( enc, dec );
			other.encrypt( plain, enc2 );
			c.Check( !std::memcmp( plain, dec, 8 ), "conversion.ice.round-trip", "level %d", level );
			c.Check( std::memcmp( enc, enc2, 8 ) != 0 && std::memcmp( enc, plain, 8 ) != 0, "conversion.ice.key-sensitivity", "level %d", level );
		}
		c.Check( ice.blockSize() == 8 && ice.keySize() == 8 * ( level ? level : 1 ), "conversion.ice.sizes", "level %d key %d", level, ice.keySize() );
	}
}

} // namespace

void RunConversionSection( Checks &c )
{
	RunFloat16( c );
	RunCompressedQuaternions( c );
	RunTransferFunctions( c );
	RunHaltonAndIce( c );
}

} // namespace mathconf
