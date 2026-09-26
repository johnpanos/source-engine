//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared harness for the mathlib conformance suite (mathlib.core).
//
// Each section checks one family of mathlib operations against an independent
// double-precision reference or an exact property, through a checker that
// takes the operation as a parameter. The same checker then runs against
// deliberately wrong operations ("bad providers") in a quiet scratch Checks,
// and the suite requires every bad provider to be rejected.
//
// Tolerances are per operation. Each one is the contract the operation's
// callers rely on, not the error one compiler happens to produce: products
// build with -ffast-math, and the suite runs under the product flags of every
// declared profile (x86-64 gcc -march=core2, arm64 NDK clang) as well as the
// strict headless flags, so a tolerance may never be tuned to one build.
//
//=============================================================================//
#ifndef MATHLIB_CONFORMANCE_H
#define MATHLIB_CONFORMANCE_H

#include "mathlib/mathlib.h"
#include "mathlib/vector.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>

namespace mathconf
{

// Counted checks. A failing check prints the first divergence of its name
// only, so one broken operation cannot flood the log.
class Checks
{
public:
	explicit Checks( bool bQuiet = false ) : m_bQuiet( bQuiet ) {}

	bool Check( bool bOk, const char *pszName, const char *pszFmt = nullptr, ... )
		__attribute__( ( format( printf, 4, 5 ) ) );

	// |got - want| <= tol.
	bool Near( const char *pszName, double got, double want, double tol );

	// Records an error sample for a named statistic without counting a check;
	// Finish() turns each statistic into one check against its budget.
	void Sample( const char *pszName, double err, double budget, const char *pszCase );

	// Converts accumulated statistics into checks. Call once per section.
	void FinishStats();

	unsigned long checks = 0;
	unsigned long failures = 0;

	bool Quiet() const { return m_bQuiet; }

private:
	struct Stat
	{
		std::string name;
		double maxErr = 0.0;
		double budget = 0.0;
		unsigned long samples = 0;
		unsigned long over = 0;
		std::string worstCase;
	};
	Stat *FindStat( const char *pszName );

	bool m_bQuiet;
	std::string m_lastFailedName;
	Stat m_stats[ 128 ];
	int m_nStats = 0;
};

// Prints each statistic's worst error when set (--stats).
extern bool g_bPrintStats;

// Deterministic generator (splitmix64); the suite never uses rand().
class Rng
{
public:
	explicit Rng( uint64_t seed ) : m_state( seed ) {}
	uint64_t Next()
	{
		uint64_t z = ( m_state += 0x9E3779B97F4A7C15ull );
		z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ull;
		z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBull;
		return z ^ ( z >> 31 );
	}
	// Uniform in [lo, hi).
	float Float( float lo, float hi ) { return lo + ( hi - lo ) * (float)( ( Next() >> 40 ) * ( 1.0 / 16777216.0 ) ); }
	double Double( double lo, double hi ) { return lo + ( hi - lo ) * ( ( Next() >> 11 ) * ( 1.0 / 9007199254740992.0 ) ); }
	int Int( int lo, int hi ) { return lo + (int)( Next() % (uint64_t)( hi - lo + 1 ) ); }
	Vector Vec( float lo, float hi ) { return Vector( Float( lo, hi ), Float( lo, hi ), Float( lo, hi ) ); }
	Vector UnitVec();
	QAngle Angles( float pitchLimit = 89.0f );
	Quaternion UnitQuat();

private:
	uint64_t m_state;
};

extern uint64_t g_nSeed;

// Double-precision reference types, independent of mathlib.
struct DVec
{
	double x, y, z;
};
struct DMat // 3x4: rotation columns + translation, like matrix3x4_t
{
	double m[ 3 ][ 4 ];
};

DMat DMatFrom( const matrix3x4_t &m );
DMat DMatMul( const DMat &a, const DMat &b );
DVec DMatApply( const DMat &a, const DVec &v ); // rotation and translation
DMat DRotationFromAngles( double pitchDeg, double yawDeg, double rollDeg );
DMat DRotationFromQuat( double x, double y, double z, double w ); // normalizes first
double DMatMaxDiff( const DMat &a, const matrix3x4_t &b );
inline DVec DV( const Vector &v ) { return DVec{ v.x, v.y, v.z }; }
inline double DMaxDiff( const DVec &a, const Vector &b )
{
	return std::fmax( std::fabs( a.x - b.x ), std::fmax( std::fabs( a.y - b.y ), std::fabs( a.z - b.z ) ) );
}

std::string Fmt( const char *pszFmt, ... ) __attribute__( ( format( printf, 1, 2 ) ) );

// Runs a checker against a bad provider in a quiet scratch Checks and counts
// one check that it was rejected.
void ExpectRejected( Checks &c, const char *pszName, const std::function< void( Checks & ) > &checker );

// Sections. Each adds checks to c; bad-provider controls are in each section.
void RunScalarSection( Checks &c );
void RunTransformSection( Checks &c );
void RunGeometrySection( Checks &c );
void RunConversionSection( Checks &c );
void RunSimdSection( Checks &c );
void RunVMatrixSection( Checks &c );

} // namespace mathconf

#endif // MATHLIB_CONFORMANCE_H
