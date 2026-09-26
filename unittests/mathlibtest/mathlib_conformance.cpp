//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entry point and harness of the mathlib conformance suite
//          (mathlib.core; unittests/mathlibtest/mathlib_conformance.h).
//
// Usage: mathlibconformance [--seed N] [--stats] [--section name]
//
// Building with -DMATHLIB_CONFORMANCE_SEED_DEFECT binds ConcatTransforms to
// a bad provider (operands swapped); the manifest's sensitivity row requires
// that to fail.
//
//=============================================================================//

#include "mathlib_conformance.h"

#include "testing/conformance_result.h"

#include <cstdarg>
#include <cstdlib>

namespace mathconf
{
bool g_bPrintStats = false;
uint64_t g_nSeed = 0x5EED0001ull;

bool Checks::Check( bool bOk, const char *pszName, const char *pszFmt, ... )
{
	++checks;
	if ( bOk )
		return true;
	++failures;
	if ( !m_bQuiet && m_lastFailedName != pszName )
	{
		m_lastFailedName = pszName;
		std::printf( "FAIL %s", pszName );
		if ( pszFmt )
		{
			std::printf( " " );
			va_list args;
			va_start( args, pszFmt );
			std::vprintf( pszFmt, args );
			va_end( args );
		}
		std::printf( "\n" );
	}
	return false;
}

bool Checks::Near( const char *pszName, double got, double want, double tol )
{
	double err = std::fabs( got - want );
	return Check( err <= tol && !std::isnan( got ), pszName, "got %.9g want %.9g err %.3g tol %.3g", got, want, err, tol );
}

Checks::Stat *Checks::FindStat( const char *pszName )
{
	for ( int i = 0; i < m_nStats; ++i )
	{
		if ( m_stats[ i ].name == pszName )
			return &m_stats[ i ];
	}
	if ( m_nStats == (int)( sizeof( m_stats ) / sizeof( m_stats[ 0 ] ) ) )
	{
		std::printf( "FAIL harness.stat-capacity %s\n", pszName );
		std::abort();
	}
	m_stats[ m_nStats ] = Stat();
	m_stats[ m_nStats ].name = pszName;
	return &m_stats[ m_nStats++ ];
}

void Checks::Sample( const char *pszName, double err, double budget, const char *pszCase )
{
	Stat *s = FindStat( pszName );
	s->budget = budget;
	++s->samples;
	if ( std::isnan( err ) )
		err = HUGE_VAL;
	if ( err > budget )
		++s->over;
	if ( err > s->maxErr || s->samples == 1 )
	{
		s->maxErr = err;
		s->worstCase = pszCase ? pszCase : "";
	}
}

void Checks::FinishStats()
{
	for ( int i = 0; i < m_nStats; ++i )
	{
		const Stat &s = m_stats[ i ];
		if ( g_bPrintStats && !m_bQuiet )
			std::printf( "STAT %-44s max %.3e budget %.3e samples %lu\n", s.name.c_str(), s.maxErr, s.budget, s.samples );
		Check( s.samples > 0 && s.over == 0, s.name.c_str(), "%lu of %lu samples over budget %.3g; worst %.3g at %s",
			s.over, s.samples, s.budget, s.maxErr, s.worstCase.c_str() );
	}
	m_nStats = 0;
}

void ExpectRejected( Checks &c, const char *pszName, const std::function< void( Checks & ) > &checker )
{
	Checks scratch( true );
	checker( scratch );
	scratch.FinishStats();
	c.Check( scratch.checks > 0 && scratch.failures > 0, pszName, "bad provider passed %lu checks", scratch.checks );
}

std::string Fmt( const char *pszFmt, ... )
{
	char buf[ 512 ];
	va_list args;
	va_start( args, pszFmt );
	std::vsnprintf( buf, sizeof( buf ), pszFmt, args );
	va_end( args );
	return buf;
}

Vector Rng::UnitVec()
{
	for ( ;; )
	{
		Vector v = Vec( -1.0f, 1.0f );
		float l2 = v.x * v.x + v.y * v.y + v.z * v.z;
		if ( l2 > 0.01f && l2 <= 1.0f )
			return v * ( 1.0f / std::sqrt( l2 ) );
	}
}

QAngle Rng::Angles( float pitchLimit )
{
	return QAngle( Float( -pitchLimit, pitchLimit ), Float( -180.0f, 180.0f ), Float( -180.0f, 180.0f ) );
}

Quaternion Rng::UnitQuat()
{
	for ( ;; )
	{
		double x = Double( -1, 1 ), y = Double( -1, 1 ), z = Double( -1, 1 ), w = Double( -1, 1 );
		double l2 = x * x + y * y + z * z + w * w;
		if ( l2 > 0.01 && l2 <= 1.0 )
		{
			double s = 1.0 / std::sqrt( l2 );
			return Quaternion( (float)( x * s ), (float)( y * s ), (float)( z * s ), (float)( w * s ) );
		}
	}
}

DMat DMatFrom( const matrix3x4_t &m )
{
	DMat r;
	for ( int i = 0; i < 3; ++i )
		for ( int j = 0; j < 4; ++j )
			r.m[ i ][ j ] = m[ i ][ j ];
	return r;
}

DMat DMatMul( const DMat &a, const DMat &b )
{
	DMat r;
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			double s = ( j == 3 ) ? a.m[ i ][ 3 ] : 0.0;
			for ( int k = 0; k < 3; ++k )
				s += a.m[ i ][ k ] * b.m[ k ][ j ];
			r.m[ i ][ j ] = s;
		}
	}
	return r;
}

DVec DMatApply( const DMat &a, const DVec &v )
{
	double in[ 3 ] = { v.x, v.y, v.z };
	double out[ 3 ];
	for ( int i = 0; i < 3; ++i )
		out[ i ] = a.m[ i ][ 0 ] * in[ 0 ] + a.m[ i ][ 1 ] * in[ 1 ] + a.m[ i ][ 2 ] * in[ 2 ] + a.m[ i ][ 3 ];
	return DVec{ out[ 0 ], out[ 1 ], out[ 2 ] };
}

// Source angles: pitch rotates about +y (positive looks down), yaw about +z,
// roll about +x, applied roll first: R = Rz(yaw) * Ry(pitch) * Rx(roll).
// Columns are forward, left and up.
DMat DRotationFromAngles( double pitchDeg, double yawDeg, double rollDeg )
{
	const double k = M_PI / 180.0;
	double sp = std::sin( pitchDeg * k ), cp = std::cos( pitchDeg * k );
	double sy = std::sin( yawDeg * k ), cy = std::cos( yawDeg * k );
	double sr = std::sin( rollDeg * k ), cr = std::cos( rollDeg * k );
	DMat rz = { { { cy, -sy, 0, 0 }, { sy, cy, 0, 0 }, { 0, 0, 1, 0 } } };
	DMat ry = { { { cp, 0, sp, 0 }, { 0, 1, 0, 0 }, { -sp, 0, cp, 0 } } };
	DMat rx = { { { 1, 0, 0, 0 }, { 0, cr, -sr, 0 }, { 0, sr, cr, 0 } } };
	return DMatMul( rz, DMatMul( ry, rx ) );
}

DMat DRotationFromQuat( double x, double y, double z, double w )
{
	double n = std::sqrt( x * x + y * y + z * z + w * w );
	x /= n;
	y /= n;
	z /= n;
	w /= n;
	DMat r = { { { 1 - 2 * ( y * y + z * z ), 2 * ( x * y - z * w ), 2 * ( x * z + y * w ), 0 },
		{ 2 * ( x * y + z * w ), 1 - 2 * ( x * x + z * z ), 2 * ( y * z - x * w ), 0 },
		{ 2 * ( x * z - y * w ), 2 * ( y * z + x * w ), 1 - 2 * ( x * x + y * y ), 0 } } };
	return r;
}

double DMatMaxDiff( const DMat &a, const matrix3x4_t &b )
{
	double d = 0.0;
	for ( int i = 0; i < 3; ++i )
		for ( int j = 0; j < 4; ++j )
			d = std::fmax( d, std::isnan( b[ i ][ j ] ) ? HUGE_VAL : std::fabs( a.m[ i ][ j ] - b[ i ][ j ] ) );
	return d;
}

} // namespace mathconf

int main( int argc, char **argv )
{
	using namespace mathconf;
	const char *pszSection = nullptr;
	for ( int i = 1; i < argc; ++i )
	{
		if ( !std::strcmp( argv[ i ], "--stats" ) )
			g_bPrintStats = true;
		else if ( !std::strcmp( argv[ i ], "--seed" ) && i + 1 < argc )
			g_nSeed = std::strtoull( argv[ ++i ], nullptr, 0 );
		else if ( !std::strcmp( argv[ i ], "--section" ) && i + 1 < argc )
			pszSection = argv[ ++i ];
		else
		{
			std::fprintf( stderr, "usage: %s [--seed N] [--stats] [--section name]\n", argv[ 0 ] );
			return 2;
		}
	}

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2 );

	struct
	{
		const char *name;
		void ( *run )( Checks & );
	} sections[] = {
		{ "scalar", RunScalarSection },
		{ "transform", RunTransformSection },
		{ "geometry", RunGeometrySection },
		{ "conversion", RunConversionSection },
		{ "simd", RunSimdSection },
		{ "vmatrix", RunVMatrixSection },
	};

	Checks c;
	for ( const auto &s : sections )
	{
		if ( pszSection && std::strcmp( pszSection, s.name ) )
			continue;
		unsigned long before = c.checks, beforeFail = c.failures;
		s.run( c );
		c.FinishStats();
		std::printf( "section %-10s %6lu checks %lu failures\n", s.name, c.checks - before, c.failures - beforeFail );
	}
	return testing::ReportConformance( c.checks, c.failures );
}
