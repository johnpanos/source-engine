//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: NaN and finiteness tests under the products' floating-point flags.
//
// Release engine targets build with -O2 -funsafe-math-optimizations
// -ftree-vectorize -ffast-math (scripts/waifulib/compiler_optimizations.py).
// -ffast-math implies -ffinite-math-only, under which GCC and clang fold
// isnan() to false and isfinite() to true, and compile a == b without a
// parity check, so a NaN compares equal to everything. Code that relies on
// NaN semantics (an Invalidate()d cache, an x != x test, an isfinite input
// check) silently stops working in release builds.
//
// This suite checks the bit-based tests the engine uses instead:
//
//   foundation::IsNaN / IsFinite (public/foundation/float_classify.h)
//   tier0 IsFinite, Vector/QAngle IsValid after Invalidate(), IS_NAN
//   the "unset cache is a miss" guard (portalsimulation, prop_mirror)
//   AlmostEqual with an infinity and a NaN (mathlib/almostequal.cpp)
//   gamepadrumble::MotorLevel and vibrator::MixMotors on NaN and infinity
//
// Every value comes from volatile bits, so no test is constant-folded.
//
// Negative control: built with -DFASTMATH_NAN_NAIVE the helpers under test
// become the naive forms (x != x, std::isnan, std::isfinite, a plain !=);
// under the same flags that build must fail (fastmath.nan-classify.naive),
// and without -ffast-math it must pass (fastmath.nan-classify.naive-ieee),
// which shows the flags, not the checks, break the naive forms.
//
//=============================================================================//

#include "testing/conformance_result.h"

#include "foundation/float_classify.h"
#include "tier0/basetypes.h"
#include "mathlib/mathlib.h"
#include "mathlib/vector.h"

#include "inputsystem/gamepad_rumble.h"
#include "inputsystem/vibrator_policy.h"

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

// IS_NAN reads mathlib's exponent mask (mathlib/mathlib_base.cpp); this
// suite links no mathlib object, so it defines the same value.
const int nanmask = 255 << 23;

bool AlmostEqual( float a, float b, int maxUlps );

namespace
{

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool ok, const char *name )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::printf( "FAIL %s\n", name );
	}
}

// Opaque inputs: the optimizer cannot see these bit patterns.
volatile std::uint32_t g_floatBits[] = {
    0x7FC00000u, // quiet NaN
    0xFFC00000u, // negative quiet NaN
    0x7F800001u, // signalling NaN
    0x7F800000u, // +infinity
    0xFF800000u, // -infinity
    0x7F7FFFFFu, // FLT_MAX
    0x00000001u, // smallest denormal
    0x00000000u, // +0
    0x3F800000u, // 1
};
enum
{
	kQNaN,
	kNegQNaN,
	kSNaN,
	kPosInf,
	kNegInf,
	kMax,
	kDenormal,
	kZero,
	kOne,
	kFloatCount
};

volatile std::uint64_t g_doubleBits[] = {
    0x7FF8000000000000ull, // quiet NaN
    0x7FF0000000000000ull, // +infinity
    0x7FEFFFFFFFFFFFFFull, // DBL_MAX
    0x3FF0000000000000ull, // 1
};

float F( int index )
{
	std::uint32_t bits = g_floatBits[index];
	float value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

double D( int index )
{
	std::uint64_t bits = g_doubleBits[index];
	double value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

bool IsNaNKind( int index )
{
	return index == kQNaN || index == kNegQNaN || index == kSNaN;
}

bool IsFiniteKind( int index )
{
	return index >= kMax;
}

//-----------------------------------------------------------------------------
// The tests under test, or their naive forms for the negative control.
//-----------------------------------------------------------------------------
#if defined( FASTMATH_NAN_NAIVE )
bool TestIsNaN( const float &v )
{
	return v != v;
}
bool TestIsNaN( const double &v )
{
	return std::isnan( v );
}
bool TestIsFinite( const float &v )
{
	return std::isfinite( v );
}
bool TestIsFinite( const double &v )
{
	return std::isfinite( v );
}
bool TestTier0IsFinite( const float &v )
{
	return std::isfinite( v );
}
bool TestIsNanMacro( const float &v )
{
	return std::isnan( v ) || std::isinf( v );
}
bool TestVectorValid( const Vector &v )
{
	return v == v;
}
bool TestAngleValid( const QAngle &q )
{
	return q == q;
}
bool TestCacheMiss( const Vector &cached, const Vector &now )
{
	return cached != now;
}
#else
bool TestIsNaN( const float &v )
{
	return foundation::IsNaN( v );
}
bool TestIsNaN( const double &v )
{
	return foundation::IsNaN( v );
}
bool TestIsFinite( const float &v )
{
	return foundation::IsFinite( v );
}
bool TestIsFinite( const double &v )
{
	return foundation::IsFinite( v );
}
bool TestTier0IsFinite( const float &v )
{
	return IsFinite( v );
}
bool TestIsNanMacro( const float &v )
{
	float copy = v;
	return IS_NAN( copy );
}
bool TestVectorValid( const Vector &v )
{
	return v.IsValid();
}
bool TestAngleValid( const QAngle &q )
{
	return q.IsValid();
}
bool TestCacheMiss( const Vector &cached, const Vector &now )
{
	return !cached.IsValid() || cached != now;
}
#endif

void CheckScalars()
{
	char name[96];
	for ( int i = 0; i < kFloatCount; ++i )
	{
		const float value = F( i );
		std::snprintf( name, sizeof( name ), "float.isnan.%d", i );
		Check( TestIsNaN( value ) == IsNaNKind( i ), name );
		std::snprintf( name, sizeof( name ), "float.isfinite.%d", i );
		Check( TestIsFinite( value ) == IsFiniteKind( i ), name );
		std::snprintf( name, sizeof( name ), "tier0.isfinite.%d", i );
		Check( TestTier0IsFinite( value ) == IsFiniteKind( i ), name );
		// IS_NAN is true for NaN and infinity (an all-ones exponent).
		std::snprintf( name, sizeof( name ), "mathlib.is-nan-macro.%d", i );
		Check( TestIsNanMacro( value ) == !IsFiniteKind( i ), name );
	}

	Check( TestIsNaN( D( 0 ) ), "double.isnan.nan" );
	Check( !TestIsNaN( D( 1 ) ), "double.isnan.inf" );
	Check( !TestIsNaN( D( 3 ) ), "double.isnan.one" );
	Check( !TestIsFinite( D( 0 ) ), "double.isfinite.nan" );
	Check( !TestIsFinite( D( 1 ) ), "double.isfinite.inf" );
	Check( TestIsFinite( D( 2 ) ), "double.isfinite.max" );
	Check( TestIsFinite( D( 3 ) ), "double.isfinite.one" );
}

void CheckVectors()
{
	const float one = F( kOne );
	const float zero = F( kZero );

	Vector invalid;
	invalid.Invalidate();
	QAngle invalidAngle;
	invalidAngle.Invalidate();
	const Vector origin( zero, zero, zero );
	const Vector unit( one, zero, zero );

	Check( !TestVectorValid( invalid ), "vector.invalidate.not-valid" );
	Check( TestVectorValid( origin ), "vector.origin.valid" );
	Check( !TestAngleValid( invalidAngle ), "qangle.invalidate.not-valid" );
	Check( TestAngleValid( QAngle( zero, zero, zero ) ), "qangle.zero.valid" );

	// One NaN component is enough.
	Vector partial( one, zero, one );
	partial.y = F( kQNaN );
	Check( !TestVectorValid( partial ), "vector.one-nan-component.not-valid" );

	// The CPortalSimulator::MoveTo / prop_mirror guard: an Invalidate()d
	// cache is a miss for every new value, including the exact origin that
	// the naive != reports as unchanged under -ffast-math.
	Check( TestCacheMiss( invalid, origin ), "cache.invalid-vs-origin.miss" );
	Check( TestCacheMiss( invalid, unit ), "cache.invalid-vs-unit.miss" );
	Check( !TestCacheMiss( origin, origin ), "cache.same.hit" );
	Check( TestCacheMiss( origin, unit ), "cache.moved.miss" );
}

void CheckConsumers()
{
	const float nan = F( kQNaN );
	const float inf = F( kPosInf );
	const float negInf = F( kNegInf );

	// AlmostEqual: NaN is equal to nothing, even an infinity.
	Check( !AlmostEqual( inf, nan, 4 ), "almostequal.inf-nan" );
	Check( !AlmostEqual( nan, nan, 4 ), "almostequal.nan-nan" );
	Check( AlmostEqual( inf, inf, 4 ), "almostequal.inf-inf" );
	Check( AlmostEqual( F( kOne ), F( kOne ), 4 ), "almostequal.one-one" );

	// Motor speeds: NaN and infinity are off, as the policies document.
	Check( gamepadrumble::MotorLevel( nan ) == 0, "gamepad-rumble.nan-off" );
	Check( gamepadrumble::MotorLevel( F( kNegQNaN ) ) == 0, "gamepad-rumble.neg-nan-off" );
	Check( gamepadrumble::MotorLevel( inf ) == 0, "gamepad-rumble.inf-off" );
	Check( gamepadrumble::MotorLevel( negInf ) == 0, "gamepad-rumble.neg-inf-off" );
	Check( gamepadrumble::MotorLevel( F( kOne ) ) == gamepadrumble::kMaxLevel,
	    "gamepad-rumble.one-max" );
	Check( vibrator::MixMotors( nan, nan ) == 0.f, "vibrator.nan-off" );
	Check( vibrator::MixMotors( inf, F( kZero ) ) == 0.f, "vibrator.inf-off" );
	Check( vibrator::MixMotors( F( kOne ), nan ) == 1.f, "vibrator.one-and-nan" );
}

} // namespace

int main()
{
	CheckScalars();
	CheckVectors();
	CheckConsumers();
	return testing::ReportConformance( g_checks, g_failures );
}
