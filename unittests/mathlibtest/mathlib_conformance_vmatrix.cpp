//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: mathlib.core vmatrix section: 4x4 VMatrix products, general and
//          rigid inverses, vector and plane transforms, axis rotations,
//          angle recovery and the right-handed projection builders
//          (vmatrix.h, vmatrix.cpp), against double-precision references.
//
//=============================================================================//

#include "mathlib_conformance.h"

#include "mathlib/vmatrix.h"
#include "mathlib/vplane.h"

#include <cfloat>

namespace mathconf
{
namespace
{

struct D4
{
	double m[4][4];
};

D4 D4From( const VMatrix &v )
{
	D4 r;
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
			r.m[i][j] = v.m[i][j];
	return r;
}

D4 D4Mul( const D4 &a, const D4 &b )
{
	D4 r;
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
		{
			double s = 0;
			for ( int k = 0; k < 4; ++k )
				s += a.m[i][k] * b.m[k][j];
			r.m[i][j] = s;
		}
	return r;
}

double D4MaxDiff( const D4 &a, const VMatrix &b )
{
	double d = 0;
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
			d = std::fmax(
			    d, IsFiniteValue( b.m[i][j] ) ? std::fabs( a.m[i][j] - b.m[i][j] ) : DBL_MAX );
	return d;
}

VMatrix RandomGeneral( Rng &rng )
{
	// Diagonally weighted, so the inverse is well conditioned.
	VMatrix m;
	for ( int i = 0; i < 4; ++i )
		for ( int j = 0; j < 4; ++j )
			m.m[i][j] = rng.Float( -1, 1 ) + ( i == j ? 4.0f : 0.0f );
	return m;
}

VMatrix RandomRigid( Rng &rng )
{
	matrix3x4_t t;
	AngleMatrix( rng.Angles( 180.0f ), rng.Vec( -4096, 4096 ), t );
	return VMatrix( t );
}

typedef void ( *MulFn )( const VMatrix &, const VMatrix &, VMatrix & );
void BadMulTransposed( const VMatrix &a, const VMatrix &b, VMatrix &out )
{
	MatrixMultiply( a, b, out );
	out = out.Transpose();
}

void CheckMultiply( Checks &c, MulFn fn )
{
	Rng rng( g_nSeed ^ 0xD0 );
	for ( int i = 0; i < 5000; ++i )
	{
		VMatrix a = RandomGeneral( rng ), b = RandomGeneral( rng ), out;
		fn( a, b, out );
		c.Sample(
		    "vmatrix.multiply", D4MaxDiff( D4Mul( D4From( a ), D4From( b ) ), out ), 4e-6, "" );
	}
}

} // namespace

void RunVMatrixSection( Checks &c )
{
	CheckMultiply( c, MatrixMultiply );
	Rng rng( g_nSeed ^ 0xD1 );
	for ( int i = 0; i < 5000; ++i )
	{
		VMatrix a = RandomGeneral( rng ), inv;
		bool ok = MatrixInverseGeneral( a, inv );
		D4 prod = D4Mul( D4From( a ), D4From( inv ) );
		double e = 0;
		for ( int r = 0; r < 4; ++r )
			for ( int k = 0; k < 4; ++k )
				e = std::fmax( e, std::fabs( prod.m[r][k] - ( r == k ? 1.0 : 0.0 ) ) );
		c.Check( ok, "vmatrix.inverse-general.invertible" );
		c.Sample( "vmatrix.inverse-general", e, 2e-6, "" );

		VMatrix rig = RandomRigid( rng ), tr;
		MatrixInverseTR( rig, tr );
		D4 p2 = D4Mul( D4From( rig ), D4From( tr ) );
		double e2 = 0;
		for ( int r = 0; r < 4; ++r )
			for ( int k = 0; k < 4; ++k )
				e2 = std::fmax( e2,
				    std::fabs( p2.m[r][k] - ( r == k ? 1.0 : 0.0 ) ) / ( k == 3 ? 4096.0 : 1.0 ) );
		c.Sample( "vmatrix.inverse-tr", e2, 2e-6, "" );

		// Vector transforms.
		Vector v = rng.Vec( -1000, 1000 ), out;
		D4 da = D4From( a );
		Vector3DMultiplyPosition( a, v, out );
		DVec want = { da.m[0][0] * v.x + da.m[0][1] * v.y + da.m[0][2] * v.z + da.m[0][3],
		    da.m[1][0] * v.x + da.m[1][1] * v.y + da.m[1][2] * v.z + da.m[1][3],
		    da.m[2][0] * v.x + da.m[2][1] * v.y + da.m[2][2] * v.z + da.m[2][3] };
		c.Sample( "vmatrix.vector3d-multiply-position", DMaxDiff( want, out ) / 5000.0, 4e-7, "" );
		Vector3DMultiply( a, v, out );
		DVec want3 = { want.x - da.m[0][3], want.y - da.m[1][3], want.z - da.m[2][3] };
		c.Sample( "vmatrix.vector3d-multiply", DMaxDiff( want3, out ) / 5000.0, 4e-7, "" );
		Vector4D v4( v.x, v.y, v.z, rng.Float( -2, 2 ) ), o4;
		Vector4DMultiply( a, v4, o4 );
		double e4 = 0;
		for ( int r = 0; r < 4; ++r )
		{
			double s =
			    da.m[r][0] * v4.x + da.m[r][1] * v4.y + da.m[r][2] * v4.z + da.m[r][3] * v4.w;
			e4 = std::fmax( e4, std::fabs( s - o4[r] ) );
		}
		c.Sample( "vmatrix.vector4d-multiply", e4 / 5000.0, 4e-7, "" );

		// Planes: transformed points lie on the transformed plane.
		VMatrix rigid = RandomRigid( rng );
		VPlane pl( rng.UnitVec(), rng.Float( -500, 500 ) ), tp;
		rigid.TransformPlane( pl, tp );
		Vector onPlane = pl.m_Normal * pl.m_Dist, moved;
		Vector3DMultiplyPosition( rigid, onPlane, moved );
		c.Sample( "vmatrix.transform-plane", std::fabs( tp.DistTo( moved ) ) / 5000.0, 4e-7, "" );

		// Axis rotation (Rodrigues) and angle recovery.
		Vector axis = rng.UnitVec();
		float deg = rng.Float( -180, 180 );
		matrix3x4_t rot;
		MatrixBuildRotationAboutAxis( axis, deg, rot );
		double th = deg * M_PI / 180.0, cs = std::cos( th ), sn = std::sin( th );
		double ax[3] = { axis.x, axis.y, axis.z };
		DMat rd;
		for ( int r = 0; r < 3; ++r )
		{
			for ( int k = 0; k < 3; ++k )
			{
				double cross = 0;
				if ( r != k )
				{
					int o = 3 - r - k;
					double sgn = ( ( r + 1 ) % 3 == k ) ? -1 : 1;
					cross = sgn * ax[o];
				}
				rd.m[r][k] = ( r == k ? cs : 0.0 ) + ( 1 - cs ) * ax[r] * ax[k] + sn * cross;
			}
			rd.m[r][3] = 0;
		}
		c.Sample( "vmatrix.rotation-about-axis", DMatMaxDiff( rd, rot ), 4e-6, "" );

		QAngle ang = rng.Angles( 85.0f ), back;
		VMatrix am;
		am.SetupMatrixAngles( ang );
		MatrixToAngles( am, back );
		matrix3x4_t m1, m2;
		AngleMatrix( ang, m1 );
		AngleMatrix( back, m2 );
		double ae = 0;
		for ( int r = 0; r < 3; ++r )
			for ( int k = 0; k < 3; ++k )
				ae = std::fmax( ae, std::fabs( m1[r][k] - m2[r][k] ) );
		c.Sample( "vmatrix.matrix-to-angles.round-trip", ae, 2e-5, "" );

		matrix3x4_t t3;
		AngleMatrix( rng.Angles( 180.0f ), rng.Vec( -100, 100 ), t3 );
		matrix3x4_t it;
		MatrixInverseTranspose( t3, it );
		// For [R t] the inverse transpose's 3x4 part is [R 0].
		double ie = 0;
		for ( int r = 0; r < 3; ++r )
			for ( int k = 0; k < 4; ++k )
				ie = std::fmax( ie, std::fabs( it[r][k] - ( k < 3 ? t3[r][k] : 0.0f ) ) );
		c.Sample( "vmatrix.inverse-transpose-rigid", ie, 1e-5, "" );
	}

	// Right-handed projection: z = -near maps to depth 0, -far to 1, and the
	// horizontal field-of-view edge to x = 1.
	for ( int i = 0; i < 500; ++i )
	{
		double fov = rng.Double( 40, 120 ), aspect = rng.Double( 1, 2.4 ), zn = rng.Double( 1, 10 ),
		       zf = rng.Double( 100, 30000 );
		VMatrix proj;
		MatrixBuildPerspectiveX( proj, fov, aspect, zn, zf );
		double tx = std::tan( fov * M_PI / 360.0 );
		Vector nearEdge( (float)( tx * zn ), 0.0f, (float)-zn ), far( 0.0f, 0.0f, (float)-zf ), out;
		Vector3DMultiplyPositionProjective( proj, nearEdge, out );
		c.Sample( "vmatrix.perspective.near",
		    std::fmax( std::fabs( out.z ), std::fabs( out.x - 1.0 ) ), 1e-5, "" );
		Vector3DMultiplyPositionProjective( proj, far, out );
		c.Sample( "vmatrix.perspective.far", std::fabs( out.z - 1.0 ), 1e-5, "" );
	}

	ExpectRejected( c, "vmatrix.multiply.rejects-transposed",
	    []( Checks &s )
	    {
		    CheckMultiply( s, BadMulTransposed );
	    } );
}

} // namespace mathconf
