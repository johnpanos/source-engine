//========= Portal 2 port ======================================================//
//
// Purpose: SIMD math the Portal 2 paint and player code takes from the later
//          (CS:GO-era) public/mathlib/ssemath.h and ssequaternion.h. Written
//          with this tree's ssemath primitives, so every configuration of
//          ssemath.h that builds this tree supports them. The formulas are
//          the CS:GO ones; the member functions CS:GO added to FourVectors are
//          free functions here so the shared mathlib header stays unchanged.
//
//=============================================================================//

#ifndef PORTAL2_SSEMATH_COMPAT_H
#define PORTAL2_SSEMATH_COMPAT_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/ssemath.h"

// CS:GO names the result type of SIMD comparisons; it is an fltx4 mask here.
typedef fltx4 bi32x4;

// [1 0 0 0], [0 1 0 0], [0 0 1 0], [0 0 0 1]
inline const fltx4 &Portal2_SIMDIdentityRow( int nRow )
{
	static const ALIGN16 float s_Identity[4][4] ALIGN16_POST = {
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
	};
	return *reinterpret_cast<const fltx4 *>( s_Identity[nRow] );
}
// Indexed like CS:GO's extern const fltx4 g_SIMD_Identity[4].
struct Portal2SIMDIdentity_t
{
	const fltx4 &operator[]( int nRow ) const { return Portal2_SIMDIdentityRow( nRow ); }
};
static const Portal2SIMDIdentity_t g_SIMD_Identity = {};

// Replace the w component of a.
FORCEINLINE fltx4 Portal2_SetWSIMD( const fltx4 &a, const fltx4 &value )
{
	fltx4 result = a;
	SubFloat( result, 3 ) = SubFloat( value, 0 );
	return result;
}

FORCEINLINE fltx4 SetWFromXSIMD( const fltx4 &a, const fltx4 &x ) { return Portal2_SetWSIMD( a, SplatXSIMD( x ) ); }
FORCEINLINE fltx4 SetWFromYSIMD( const fltx4 &a, const fltx4 &y ) { return Portal2_SetWSIMD( a, SplatYSIMD( y ) ); }
FORCEINLINE fltx4 SetWFromZSIMD( const fltx4 &a, const fltx4 &z ) { return Portal2_SetWSIMD( a, SplatZSIMD( z ) ); }

FORCEINLINE fltx4 CrossProductSIMD( const fltx4 &A, const fltx4 &B )
{
	fltx4 CrossVal;
	SubFloat( CrossVal, 0 ) = SubFloat( A, 1 ) * SubFloat( B, 2 ) - SubFloat( A, 2 ) * SubFloat( B, 1 );
	SubFloat( CrossVal, 1 ) = SubFloat( A, 2 ) * SubFloat( B, 0 ) - SubFloat( A, 0 ) * SubFloat( B, 2 );
	SubFloat( CrossVal, 2 ) = SubFloat( A, 0 ) * SubFloat( B, 1 ) - SubFloat( A, 1 ) * SubFloat( B, 0 );
	SubFloat( CrossVal, 3 ) = 0;
	return CrossVal;
}

// Est(a/b)
FORCEINLINE fltx4 DivEstSIMD( const fltx4 &a, const fltx4 &b )
{
	return MulSIMD( ReciprocalEstSIMD( b ), a );
}

inline const fltx4 Normalized3SIMD( const fltx4 vec )
{
	fltx4 scLengthSqr = Dot3SIMD( vec, vec );
	bi32x4 isSignificant = CmpGtSIMD( scLengthSqr, Four_Epsilons );
	fltx4 scLengthInv = ReciprocalSqrtSIMD( scLengthSqr );
	return AndSIMD( isSignificant, MulSIMD( vec, scLengthInv ) );
}

FORCEINLINE fltx4 Portal2_LerpSIMD( const fltx4 &percent, const fltx4 &a, const fltx4 &b )
{
	return AddSIMD( a, MulSIMD( SubSIMD( b, a ), percent ) );
}

// Remap val from the clamped range [a, b] to [c, d].
FORCEINLINE fltx4 RemapValClampedSIMD( const fltx4 &val, const fltx4 &a, const fltx4 &b, const fltx4 &c, const fltx4 &d )
{
	fltx4 range = MaskedAssign( CmpEqSIMD( a, b ), Four_Ones, SubSIMD( b, a ) ); // make sure range > 0
	fltx4 cVal = MaxSIMD( Four_Zeros, MinSIMD( Four_Ones, DivSIMD( SubSIMD( val, a ), range ) ) ); // saturate
	return Portal2_LerpSIMD( cVal, c, d );
}

//-----------------------------------------------------------------------------
// FourVectors additions
//-----------------------------------------------------------------------------
inline FourVectors operator+( const FourVectors &a, const FourVectors &b )
{
	FourVectors ret = a;
	ret += b;
	return ret;
}

inline FourVectors operator-( const FourVectors &a, const FourVectors &b )
{
	FourVectors ret = a;
	ret -= b;
	return ret;
}

inline FourVectors Mul( const FourVectors &a, const fltx4 &b )
{
	FourVectors ret;
	ret.x = MulSIMD( a.x, b );
	ret.y = MulSIMD( a.y, b );
	ret.z = MulSIMD( a.z, b );
	return ret;
}

// CS:GO FourVectors::StoreUnalignedVector3SIMD().
inline void Portal2_StoreUnalignedVector3SIMD( const FourVectors &v, Vector *out0, Vector *out1, Vector *out2, Vector *out3 )
{
	*out0 = v.Vec( 0 );
	*out1 = v.Vec( 1 );
	*out2 = v.Vec( 2 );
	*out3 = v.Vec( 3 );
}

// CS:GO FourVectors::CalcClosestPointOnLineSIMD(): the closest point to P on
// the infinite line through vLineA and vLineB; *outT receives t for
// A + (B - A) t. A degenerate line yields t = 0 (the CS:GO code intended this
// but discarded the masked result).
inline void Portal2_CalcClosestPointOnLineSIMD( const FourVectors &P, const FourVectors &vLineA, const FourVectors &vLineB,
												FourVectors &vClosest, fltx4 *outT = NULL )
{
	FourVectors vDir = vLineB;
	vDir -= vLineA;

	fltx4 div = vDir * vDir;
	bi32x4 mask = CmpLtSIMD( div, ReplicateX4( 0.00001f ) );
	fltx4 t = DivSIMD( SubSIMD( vDir * P, vDir * vLineA ), div );
	t = MaskedAssign( mask, Four_Zeros, t );

	if ( outT )
		*outT = t;
	vClosest = vDir;
	vClosest *= t;
	vClosest += vLineA;
}

//-----------------------------------------------------------------------------
// CS:GO FourQuaternions (ssequaternion.h), the members Portal 2 uses.
//-----------------------------------------------------------------------------
class ALIGN16 FourQuaternions
{
public:
	fltx4 x, y, z, w;

	// Four rotations around the same axis; angles in radians.
	FourQuaternions &FromAxisAndAngles( const fltx4 &axis, const fltx4 &angles )
	{
		fltx4 theta = MulSIMD( angles, Four_PointFives ); // half angles
		fltx4 vsines, vcoses;
		SinCosSIMD( vsines, vcoses, theta );

		x = MulSIMD( vsines, SplatXSIMD( axis ) );
		y = MulSIMD( vsines, SplatYSIMD( axis ) );
		z = MulSIMD( vsines, SplatZSIMD( axis ) );
		w = vcoses;
		return *this;
	}

	FourQuaternions &FromAxisAndAnglesInDegrees( const fltx4 &axis, const fltx4 &angles )
	{
		return FromAxisAndAngles( axis, MulSIMD( angles, ReplicateX4( M_PI_F / 180.0f ) ) );
	}

	// Rotate (in place) four vectors, each by its own quaternion.
	void RotateFourVectors( FourVectors *vecs ) const
	{
		fltx4 tmpX = SubSIMD( MaddSIMD( w, vecs->x, MulSIMD( y, vecs->z ) ), MulSIMD( z, vecs->y ) );
		fltx4 tmpY = SubSIMD( MaddSIMD( w, vecs->y, MulSIMD( z, vecs->x ) ), MulSIMD( x, vecs->z ) );
		fltx4 tmpZ = SubSIMD( MaddSIMD( w, vecs->z, MulSIMD( x, vecs->y ) ), MulSIMD( y, vecs->x ) );
		fltx4 tmpW = AddSIMD( MaddSIMD( x, vecs->x, MulSIMD( y, vecs->y ) ), MulSIMD( z, vecs->z ) );

		fltx4 outX = AddSIMD( SubSIMD( MaddSIMD( tmpW, x, MulSIMD( tmpX, w ) ), MulSIMD( tmpY, z ) ), MulSIMD( tmpZ, y ) );
		fltx4 outY = AddSIMD( SubSIMD( MaddSIMD( tmpW, y, MulSIMD( tmpY, w ) ), MulSIMD( tmpZ, x ) ), MulSIMD( tmpX, z ) );
		fltx4 outZ = AddSIMD( SubSIMD( MaddSIMD( tmpW, z, MulSIMD( tmpZ, w ) ), MulSIMD( tmpX, y ) ), MulSIMD( tmpY, x ) );

		vecs->x = outX;
		vecs->y = outY;
		vecs->z = outZ;
	}
} ALIGN16_POST;

#endif // PORTAL2_SSEMATH_COMPAT_H
