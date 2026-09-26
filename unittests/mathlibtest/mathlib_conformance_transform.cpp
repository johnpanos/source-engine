//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: mathlib.core transform section: Euler angles, matrix3x4_t and
//          quaternions (mathlib/mathlib_base.cpp), the operations bone
//          setup, movement and collision call per frame.
//
// References are double-precision constructions independent of mathlib: a
// rotation built as Rz(yaw) Ry(pitch) Rx(roll), a quaternion's matrix from
// the textbook formula, and 3x4 products in double. Quaternions are compared
// through their rotation matrices, so either sign is accepted where the
// contract does not fix it.
//
// Budgets: rotation entries are unit-scale, so 4e-6 covers a few float ulps
// of the trigonometry plus the degree-to-radian scaling of angles up to 180.
// Translations are scaled by the operand magnitude (4096 world units).
//
//=============================================================================//

#include "mathlib_conformance.h"

#include <algorithm>
#include <cfloat>

namespace mathconf
{
namespace
{

const double kRotTol = 4e-6;
const double kWorld = 4096.0;

matrix3x4_t RandomTransform( Rng &rng, float trans = (float)kWorld )
{
	matrix3x4_t m;
	AngleMatrix( rng.Angles( 180.0f ), rng.Vec( -trans, trans ), m );
	return m;
}

DMat DTransform( const QAngle &a, const Vector &pos )
{
	DMat r = DRotationFromAngles( a.x, a.y, a.z );
	r.m[0][3] = pos.x;
	r.m[1][3] = pos.y;
	r.m[2][3] = pos.z;
	return r;
}

// Budget for a rotation rebuilt from recovered Euler angles. Recovery is
// ill-conditioned by 1/cos(pitch), and when cos(pitch) < 0.001 MatrixAngles
// takes its gimbal branch (yaw = 0), which drops terms of order cos(pitch).
double EulerRecoveryBudget( double pitchDeg )
{
	double cp = std::fabs( std::cos( pitchDeg * M_PI / 180.0 ) );
	return 2e-6 + 8e-7 / std::fmax( cp, 1e-6 ) + ( cp < 1.5e-3 ? 2.0 * cp : 0.0 );
}

// Bad providers --------------------------------------------------------------

void BadConcatSwapped( const matrix3x4_t &a, const matrix3x4_t &b, matrix3x4_t &out )
{
	ConcatTransforms( b, a, out );
}
void BadConcatNoTranslation( const matrix3x4_t &a, const matrix3x4_t &b, matrix3x4_t &out )
{
	ConcatTransforms( a, b, out );
	out[0][3] -= a[0][3];
}

void BadAngleVectorsRoll( const QAngle &a, Vector *f, Vector *r, Vector *u )
{
	AngleVectors( QAngle( a.x, a.y, -a.z ), f, r, u );
}

void BadSlerpNlerp( const Quaternion &p, const Quaternion &q, float t, Quaternion &out )
{
	QuaternionBlend( p, q, t, out );
}
void BadSlerpNoAlign( const Quaternion &p, const Quaternion &q, float t, Quaternion &out )
{
	QuaternionSlerpNoAlign( p, q, t, out );
}

void BadQuatMatrixTransposed( const Quaternion &q, matrix3x4_t &m )
{
	QuaternionMatrix( q, m );
	MatrixTranspose( m );
}

void BadMatrixInvertNoTranslation( const matrix3x4_t &in, matrix3x4_t &out )
{
	MatrixInvert( in, out );
	out[0][3] = out[1][3] = out[2][3] = 0.0f;
}

void BadTransformAABBCorners(
    const matrix3x4_t &m, const Vector &mins, const Vector &maxs, Vector &omin, Vector &omax )
{
	Vector a, b;
	VectorTransform( mins, m, a );
	VectorTransform( maxs, m, b );
	VectorMin( a, b, omin );
	VectorMax( a, b, omax );
}

// Checkers -------------------------------------------------------------------

typedef void ( *AngleVectorsFn )( const QAngle &, Vector *, Vector *, Vector * );
typedef void ( *ConcatFn )( const matrix3x4_t &, const matrix3x4_t &, matrix3x4_t & );
typedef void ( *SlerpFn )( const Quaternion &, const Quaternion &, float, Quaternion & );
typedef void ( *QuatMatrixFn )( const Quaternion &, matrix3x4_t & );
typedef void ( *InvertFn )( const matrix3x4_t &, matrix3x4_t & );
typedef void ( *AabbFn )( const matrix3x4_t &, const Vector &, const Vector &, Vector &, Vector & );

void CheckAngleVectors( Checks &c, AngleVectorsFn fn, int n )
{
	Rng rng( g_nSeed ^ 0xA1 );
	for ( int i = 0; i < n; ++i )
	{
		QAngle a = rng.Angles( 180.0f );
		if ( i < 8 )
			a = QAngle( ( i & 1 ) ? 90.0f : -90.0f, 45.0f * i, ( i & 2 ) ? 180.0f : 0.0f );
		Vector f, r, u;
		fn( a, &f, &r, &u );
		DMat R = DRotationFromAngles( a.x, a.y, a.z );
		std::string cs = Fmt( "(%g %g %g)", a.x, a.y, a.z );
		c.Sample( "angles.angle-vectors.forward",
		    DMaxDiff( DVec{ R.m[0][0], R.m[1][0], R.m[2][0] }, f ), kRotTol, cs.c_str() );
		c.Sample( "angles.angle-vectors.right",
		    DMaxDiff( DVec{ -R.m[0][1], -R.m[1][1], -R.m[2][1] }, r ), kRotTol, cs.c_str() );
		c.Sample( "angles.angle-vectors.up", DMaxDiff( DVec{ R.m[0][2], R.m[1][2], R.m[2][2] }, u ),
		    kRotTol, cs.c_str() );
	}
}

void CheckConcat( Checks &c, ConcatFn fn, int n )
{
	Rng rng( g_nSeed ^ 0xC0 );
	for ( int i = 0; i < n; ++i )
	{
		matrix3x4_t a = RandomTransform( rng ), b = RandomTransform( rng ), out;
		if ( i & 1 )
		{
			// General (non-rigid) operands: callers concatenate scaled bones.
			for ( int r = 0; r < 3; ++r )
				for ( int k = 0; k < 3; ++k )
					a[r][k] *= rng.Float( 0.25f, 4.0f );
		}
		DMat want = DMatMul( DMatFrom( a ), DMatFrom( b ) );
		fn( a, b, out );
		double rot = 0, tr = 0;
		for ( int r = 0; r < 3; ++r )
		{
			for ( int k = 0; k < 3; ++k )
				rot = std::fmax( rot, std::fabs( want.m[r][k] - out[r][k] ) );
			tr = std::fmax( tr, std::fabs( want.m[r][3] - out[r][3] ) );
		}
		c.Sample( "matrix.concat.rotation", rot, 1e-5 * 16, "random" );
		c.Sample( "matrix.concat.translation", tr, kWorld * 4 * 2e-6, "random" );
	}
	// Aliasing: out may be either operand.
	matrix3x4_t a = RandomTransform( rng ), b = RandomTransform( rng ), ref, alias;
	fn( a, b, ref );
	alias = a;
	fn( alias, b, alias );
	c.Check( MatricesAreEqual( ref, alias, 0.0f ), "matrix.concat.alias-first" );
	alias = b;
	fn( a, alias, alias );
	c.Check( MatricesAreEqual( ref, alias, 0.0f ), "matrix.concat.alias-second" );
}

void CheckSlerp( Checks &c, SlerpFn fn, int n )
{
	Rng rng( g_nSeed ^ 0x51 );
	for ( int i = 0; i < n; ++i )
	{
		Quaternion p = rng.UnitQuat(), q = rng.UnitQuat();
		float t = rng.Float( 0.0f, 1.0f );
		if ( i % 16 == 0 )
			t = ( i % 32 ) ? 1.0f : 0.0f;
		Quaternion out;
		fn( p, q, t, out );

		// Reference: shortest-arc slerp in double.
		double pd[4] = { p.x, p.y, p.z, p.w }, qd[4] = { q.x, q.y, q.z, q.w };
		double dot = pd[0] * qd[0] + pd[1] * qd[1] + pd[2] * qd[2] + pd[3] * qd[3];
		if ( dot < 0 )
		{
			for ( double &v : qd )
				v = -v;
			dot = -dot;
		}
		dot = std::min( dot, 1.0 );
		double om = std::acos( dot ), sp, sq;
		if ( om < 1e-6 )
			sp = 1 - t, sq = t;
		else
			sp = std::sin( ( 1 - t ) * om ) / std::sin( om ),
			sq = std::sin( t * om ) / std::sin( om );
		double r[4];
		for ( int k = 0; k < 4; ++k )
			r[k] = sp * pd[k] + sq * qd[k];
		double s = ( r[0] * out.x + r[1] * out.y + r[2] * out.z + r[3] * out.w ) < 0 ? -1 : 1;
		double err =
		    std::fmax( std::fmax( std::fabs( s * r[0] - out.x ), std::fabs( s * r[1] - out.y ) ),
		        std::fmax( std::fabs( s * r[2] - out.z ), std::fabs( s * r[3] - out.w ) ) );
		// acos near dot=1 loses precision in float: the budget is 1e-4 in
		// quaternion components (about 0.01 degrees), far below bone jitter.
		c.Sample( "quat.slerp.shortest-arc", err, 1e-4, Fmt( "dot %.6f t %.3f", dot, t ).c_str() );
		double len = std::sqrt( (double)out.x * out.x + (double)out.y * out.y +
		                        (double)out.z * out.z + (double)out.w * out.w );
		c.Sample( "quat.slerp.unit", std::fabs( len - 1.0 ), 1e-5, "" );
	}
}

void CheckQuaternionMatrix( Checks &c, QuatMatrixFn fn, int n )
{
	Rng rng( g_nSeed ^ 0x4D );
	for ( int i = 0; i < n; ++i )
	{
		Quaternion q = rng.UnitQuat();
		matrix3x4_t m;
		fn( q, m );
		DMat want = DRotationFromQuat( q.x, q.y, q.z, q.w );
		c.Sample( "quat.matrix", DMatMaxDiff( want, m ), kRotTol,
		    Fmt( "(%g %g %g %g)", q.x, q.y, q.z, q.w ).c_str() );
	}
}

void CheckMatrixInvert( Checks &c, InvertFn fn, int n )
{
	Rng rng( g_nSeed ^ 0x1B );
	for ( int i = 0; i < n; ++i )
	{
		matrix3x4_t m = RandomTransform( rng ), inv;
		fn( m, inv );
		DMat prod = DMatMul( DMatFrom( inv ), DMatFrom( m ) );
		DMat ident = { { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 } } };
		double err = 0;
		for ( int r = 0; r < 3; ++r )
			for ( int k = 0; k < 4; ++k )
				err = std::fmax(
				    err, std::fabs( prod.m[r][k] - ident.m[r][k] ) / ( k == 3 ? kWorld : 1.0 ) );
		c.Sample( "matrix.invert-rigid", err, 2e-6, "" );
	}
	// In place.
	matrix3x4_t m = RandomTransform( rng ), a, b = m;
	fn( m, a );
	fn( b, b );
	c.Check( MatricesAreEqual( a, b, 0.0f ), "matrix.invert-rigid.in-place" );
}

void CheckTransformAABB( Checks &c, AabbFn fn, bool bInverse, int n )
{
	Rng rng( g_nSeed ^ 0xAB );
	const char *pszName = bInverse ? "aabb.itransform" : "aabb.transform";
	for ( int i = 0; i < n; ++i )
	{
		matrix3x4_t m = RandomTransform( rng, 1024.0f );
		Vector a = rng.Vec( -512, 512 ), b = rng.Vec( -512, 512 );
		Vector mins, maxs, omin, omax;
		VectorMin( a, b, mins );
		VectorMax( a, b, maxs );
		fn( m, mins, maxs, omin, omax );
		DMat dm = DMatFrom( m );
		if ( bInverse )
		{
			// Inverse of a rigid transform: transpose rotation, -R^T t.
			DMat inv;
			for ( int r = 0; r < 3; ++r )
				for ( int k = 0; k < 3; ++k )
					inv.m[r][k] = dm.m[k][r];
			for ( int r = 0; r < 3; ++r )
				inv.m[r][3] = -( inv.m[r][0] * dm.m[0][3] + inv.m[r][1] * dm.m[1][3] +
				                 inv.m[r][2] * dm.m[2][3] );
			dm = inv;
		}
		DVec lo = { DBL_MAX, DBL_MAX, DBL_MAX }, hi = { -DBL_MAX, -DBL_MAX, -DBL_MAX };
		for ( int k = 0; k < 8; ++k )
		{
			DVec p = DMatApply( dm, DVec{ ( k & 1 ) ? maxs.x : mins.x, ( k & 2 ) ? maxs.y : mins.y,
			                            ( k & 4 ) ? maxs.z : mins.z } );
			lo = DVec{ std::min( lo.x, p.x ), std::min( lo.y, p.y ), std::min( lo.z, p.z ) };
			hi = DVec{ std::max( hi.x, p.x ), std::max( hi.y, p.y ), std::max( hi.z, p.z ) };
		}
		double err = std::fmax( DMaxDiff( lo, omin ), DMaxDiff( hi, omax ) );
		c.Sample( pszName, err, 2048 * 8e-7 * 4, "" );
	}
}

// Sections ---------------------------------------------------------------------

void RunAngles( Checks &c )
{
	CheckAngleVectors( c, AngleVectors, 4000 );

	Rng rng( g_nSeed ^ 0xA2 );
	for ( int i = 0; i < 2000; ++i )
	{
		QAngle a = rng.Angles( 180.0f );
		Vector pos = rng.Vec( -kWorld, kWorld );
		matrix3x4_t m;
		AngleMatrix( a, pos, m );
		DMat want = DTransform( a, pos );
		double rot = 0, tr = 0;
		for ( int r = 0; r < 3; ++r )
		{
			for ( int k = 0; k < 3; ++k )
				rot = std::fmax( rot, std::fabs( want.m[r][k] - m[r][k] ) );
			tr = std::fmax( tr, std::fabs( want.m[r][3] - m[r][3] ) );
		}
		c.Sample( "angles.angle-matrix", rot, kRotTol, "" );
		c.Sample( "angles.angle-matrix.position", tr, 0.0, "" ); // copied, exact

		// AngleIMatrix is the transpose.
		matrix3x4_t im;
		AngleIMatrix( a, im );
		double terr = 0;
		for ( int r = 0; r < 3; ++r )
			for ( int k = 0; k < 3; ++k )
				terr = std::fmax( terr, std::fabs( want.m[k][r] - im[r][k] ) );
		c.Sample( "angles.angle-imatrix", terr, kRotTol, "" );

		// MatrixAngles recovers a rotation equal to the input (angles
		// themselves are not unique at |pitch| = 90).
		QAngle back;
		MatrixAngles( m, back );
		matrix3x4_t m2;
		AngleMatrix( back, m2 );
		double rt = 0;
		for ( int r = 0; r < 3; ++r )
			for ( int k = 0; k < 3; ++k )
				rt = std::fmax( rt, std::fabs( m[r][k] - m2[r][k] ) );
		// Near gimbal lock (|pitch| -> 90) the float recovery of roll/yaw
		// degrades; outside 89.9 degrees the contract is 2e-5.
		if ( std::fabs( a.x ) < 89.9f )
			c.Sample( "angles.matrix-angles.round-trip", rt, 2e-5,
			    Fmt( "(%g %g %g)", a.x, a.y, a.z ).c_str() );

		// AngleQuaternion is the same rotation.
		Quaternion q;
		AngleQuaternion( a, q );
		DMat qm = DRotationFromQuat( q.x, q.y, q.z, q.w );
		double qe = 0;
		for ( int r = 0; r < 3; ++r )
			for ( int k = 0; k < 3; ++k )
				qe = std::fmax( qe, std::fabs( qm.m[r][k] - want.m[r][k] ) );
		c.Sample( "angles.angle-quaternion", qe, kRotTol, "" );

		QAngle qa;
		QuaternionAngles( q, qa );
		matrix3x4_t m3;
		AngleMatrix( qa, m3 );
		double qae = 0;
		for ( int r = 0; r < 3; ++r )
			for ( int k = 0; k < 3; ++k )
				qae = std::fmax( qae, std::fabs( m3[r][k] - want.m[r][k] ) );
		c.Sample( "angles.quaternion-angles.round-trip", qae / EulerRecoveryBudget( a.x ), 1.0,
		    Fmt( "(%g %g %g)", a.x, a.y, a.z ).c_str() );

		// RadianEuler forms agree with the degree forms.
		RadianEuler re( a );
		Quaternion q2;
		AngleQuaternion( re, q2 );
		c.Sample( "angles.radian-euler-quaternion",
		    std::fabs( QuaternionDotProduct( q, q2 ) ) > 0
		        ? 1.0 - std::fabs( QuaternionDotProduct( q, q2 ) )
		        : 1.0,
		    2e-6, "" );
	}

	// VectorAngles: AngleVectors of the result is the normalized input.
	for ( int i = 0; i < 2000; ++i )
	{
		Vector f = rng.Vec( -100, 100 );
		if ( i < 4 )
			f = Vector( 0, 0, ( i & 1 ) ? 5.0f : -5.0f );
		QAngle a;
		VectorAngles( f, a );
		Vector g;
		AngleVectors( a, &g );
		Vector fn = f;
		VectorNormalize( fn );
		c.Sample( "angles.vector-angles.round-trip", ( g - fn ).Length(), 1e-5, "" );
		c.Check( a.x >= -90.0f && a.x < 360.0f && a.y >= 0.0f && a.y < 360.0f && a.z == 0.0f,
		    "angles.vector-angles.range", "(%g %g %g)", a.x, a.y, a.z );
	}

	// VectorVectors: right and up complete an orthonormal basis with forward.
	for ( int i = 0; i < 1000; ++i )
	{
		Vector f = ( i < 2 ) ? Vector( 0, 0, i ? 1.0f : -1.0f ) : rng.UnitVec();
		Vector r, u;
		VectorVectors( f, r, u );
		double e = std::fmax( std::fabs( DotProduct( f, r ) ),
		    std::fmax( std::fabs( DotProduct( f, u ) ), std::fabs( DotProduct( r, u ) ) ) );
		e = std::fmax( e, std::fmax( std::fabs( r.Length() - 1 ), std::fabs( u.Length() - 1 ) ) );
		// cross( forward, z ) shrinks with forward's horizontal length, which
		// scales the input's float error in the normalized result.
		double h = std::sqrt( (double)f.x * f.x + (double)f.y * f.y );
		double vvBudget = 4e-6 + ( h > 0 ? 4e-7 / h : 0.0 );
		c.Sample( "angles.vector-vectors.orthonormal", e / vvBudget, 1.0,
		    Fmt( "horizontal %g", h ).c_str() );
		Vector cross = CrossProduct( r, f );
		c.Sample( "angles.vector-vectors.handedness", ( cross - u ).Length() / ( 2.5 * vvBudget ),
		    1.0, Fmt( "horizontal %g", h ).c_str() );
	}

	ExpectRejected( c, "angles.angle-vectors.rejects-negated-roll",
	    []( Checks &s )
	    {
		    CheckAngleVectors( s, BadAngleVectorsRoll, 200 );
	    } );
}

void RunMatrices( Checks &c )
{
#ifdef MATHLIB_CONFORMANCE_SEED_DEFECT
	CheckConcat( c, BadConcatSwapped, 4000 );
#else
	CheckConcat( c, ConcatTransforms, 4000 );
#endif
	CheckMatrixInvert( c, MatrixInvert, 2000 );
	CheckTransformAABB( c, TransformAABB, false, 2000 );
	CheckTransformAABB( c, ITransformAABB, true, 2000 );

	Rng rng( g_nSeed ^ 0x77 );
	for ( int i = 0; i < 2000; ++i )
	{
		matrix3x4_t m = RandomTransform( rng );
		Vector v = rng.Vec( -kWorld, kWorld ), out;
		DMat dm = DMatFrom( m );
		DVec want = DMatApply( dm, DV( v ) );
		VectorTransform( v, m, out );
		const double posTol = kWorld * 3 * 4e-7 * 4;
		c.Sample( "matrix.vector-transform", DMaxDiff( want, out ), posTol, "" );

		Vector back;
		VectorITransform( out, m, back );
		c.Sample( "matrix.vector-itransform.round-trip", ( back - v ).Length(), posTol * 2, "" );

		DVec rot = { dm.m[0][0] * v.x + dm.m[0][1] * v.y + dm.m[0][2] * v.z,
		    dm.m[1][0] * v.x + dm.m[1][1] * v.y + dm.m[1][2] * v.z,
		    dm.m[2][0] * v.x + dm.m[2][1] * v.y + dm.m[2][2] * v.z };
		VectorRotate( v, m, out );
		c.Sample( "matrix.vector-rotate", DMaxDiff( rot, out ), posTol, "" );
		VectorIRotate( out, m, back );
		c.Sample( "matrix.vector-irotate.round-trip", ( back - v ).Length(), posTol * 2, "" );

		// RotateAABB / IRotateAABB ignore translation: compare against the
		// transform variants of a translation-free copy.
		matrix3x4_t r = m;
		r[0][3] = r[1][3] = r[2][3] = 0.0f;
		Vector a = rng.Vec( -512, 512 ), b = rng.Vec( -512, 512 ), mins, maxs, o1, o2, o3, o4;
		VectorMin( a, b, mins );
		VectorMax( a, b, maxs );
		RotateAABB( m, mins, maxs, o1, o2 );
		TransformAABB( r, mins, maxs, o3, o4 );
		c.Sample(
		    "aabb.rotate", std::fmax( ( o1 - o3 ).Length(), ( o2 - o4 ).Length() ), 1e-3, "" );
		IRotateAABB( m, mins, maxs, o1, o2 );
		ITransformAABB( r, mins, maxs, o3, o4 );
		c.Sample(
		    "aabb.irotate", std::fmax( ( o1 - o3 ).Length(), ( o2 - o4 ).Length() ), 1e-3, "" );

		// MatrixTranspose. (MatrixInverseTranspose is inline in vmatrix.h and
		// checked in the vmatrix section.)
		matrix3x4_t t;
		MatrixTranspose( m, t );
		bool bT = true;
		for ( int rr = 0; rr < 3; ++rr )
			for ( int k = 0; k < 3; ++k )
				bT = bT && t[rr][k] == m[k][rr];
		c.Check( bT, "matrix.transpose" );
	}

	// MatrixAngles( mat, q, pos ) / MatrixQuaternion / QuaternionMatrix.
	for ( int i = 0; i < 2000; ++i )
	{
		matrix3x4_t m = RandomTransform( rng );
		Quaternion q;
		MatrixQuaternion( m, q );
		matrix3x4_t m2;
		QuaternionMatrix( q, m2 );
		double e = 0;
		for ( int r = 0; r < 3; ++r )
			for ( int k = 0; k < 3; ++k )
				e = std::fmax( e, std::fabs( m[r][k] - m2[r][k] ) );
		// MatrixQuaternion goes through MatrixAngles, so it inherits the
		// Euler recovery's 1/cos(pitch) conditioning (normalized error).
		QAngle ma;
		MatrixAngles( m, ma );
		c.Sample( "quat.matrix-quaternion.round-trip", e / EulerRecoveryBudget( ma.x ), 1.0,
		    Fmt( "pitch %g", ma.x ).c_str() );
		Vector pos;
		Quaternion q2;
		MatrixAngles( m, q2, pos );
		c.Sample( "quat.matrix-angles-quaternion", 1.0 - std::fabs( QuaternionDotProduct( q, q2 ) ),
		    2e-6, "" );
		c.Check( pos.x == m[0][3] && pos.y == m[1][3] && pos.z == m[2][3],
		    "matrix.matrix-angles.position" );
	}

	ExpectRejected( c, "matrix.concat.rejects-swapped-operands",
	    []( Checks &s )
	    {
		    CheckConcat( s, BadConcatSwapped, 200 );
	    } );
	ExpectRejected( c, "matrix.concat.rejects-dropped-translation",
	    []( Checks &s )
	    {
		    CheckConcat( s, BadConcatNoTranslation, 200 );
	    } );
	ExpectRejected( c, "matrix.invert.rejects-dropped-translation",
	    []( Checks &s )
	    {
		    CheckMatrixInvert( s, BadMatrixInvertNoTranslation, 200 );
	    } );
	ExpectRejected( c, "aabb.transform.rejects-two-corner-bounds",
	    []( Checks &s )
	    {
		    CheckTransformAABB( s, BadTransformAABBCorners, false, 200 );
	    } );
}

void RunQuaternions( Checks &c )
{
	CheckSlerp( c, QuaternionSlerp, 6000 );
	CheckQuaternionMatrix( c, QuaternionMatrix, 3000 );

	Rng rng( g_nSeed ^ 0x9A );
	for ( int i = 0; i < 3000; ++i )
	{
		Quaternion p = rng.UnitQuat(), q = rng.UnitQuat();
		float t = rng.Float( 0, 1 );

		// QuaternionMult composes rotations: M(p*q) = M(p) M(q).
		Quaternion pq;
		QuaternionMult( p, q, pq );
		DMat want = DMatMul(
		    DRotationFromQuat( p.x, p.y, p.z, p.w ), DRotationFromQuat( q.x, q.y, q.z, q.w ) );
		matrix3x4_t m;
		QuaternionMatrix( pq, m );
		c.Sample( "quat.mult.composes", DMatMaxDiff( want, m ), 1e-5, "" );

		// QuaternionBlend: normalized lerp along the shortest arc.
		Quaternion b;
		QuaternionBlend( p, q, t, b );
		Quaternion qa;
		QuaternionAlign( p, q, qa );
		double r[4] = { ( 1 - t ) * p.x + t * qa.x, ( 1 - t ) * p.y + t * qa.y,
		    ( 1 - t ) * p.z + t * qa.z, ( 1 - t ) * p.w + t * qa.w };
		double n = std::sqrt( r[0] * r[0] + r[1] * r[1] + r[2] * r[2] + r[3] * r[3] );
		double be =
		    std::fmax( std::fmax( std::fabs( r[0] / n - b.x ), std::fabs( r[1] / n - b.y ) ),
		        std::fmax( std::fabs( r[2] / n - b.z ), std::fabs( r[3] / n - b.w ) ) );
		c.Sample( "quat.blend", be, 4e-6, "" );
		c.Check( QuaternionDotProduct( p, qa ) >= -1e-6f, "quat.align.shortest", "dot %g",
		    QuaternionDotProduct( p, qa ) );

		// QuaternionScale( p, t ): rotation about the same axis by t times the angle.
		Quaternion s;
		float tt = rng.Float( -1.0f, 1.0f );
		QuaternionScale( p, tt, s );
		Vector axis;
		float ang;
		QuaternionAxisAngle( p, axis, ang );
		Quaternion sw;
		AxisAngleQuaternion( axis, ang * tt, sw );
		c.Sample( "quat.scale", 1.0 - std::fabs( QuaternionDotProduct( s, sw ) ), 2e-5,
		    Fmt( "t %g", tt ).c_str() );

		// QuaternionAngleDiff is the rotation angle between p and q, in degrees.
		double dotpq = std::fabs(
		    (double)p.x * q.x + (double)p.y * q.y + (double)p.z * q.z + (double)p.w * q.w );
		double wantDeg = 2.0 * std::acos( std::min( dotpq, 1.0 ) ) * 180.0 / M_PI;
		// asin( |v| ) of the difference quaternion: float error in |v| is
		// amplified by 1/cos(angle/2), which diverges toward 180 degrees.
		double half = wantDeg * M_PI / 360.0;
		double cond = 1.0 / std::fmax( std::cos( half ), 1e-6 );
		double adBudget = 2e-3 + 2.0 * ( 180.0 / M_PI ) * 4e-7 * cond;
		c.Sample( "quat.angle-diff", std::fabs( QuaternionAngleDiff( p, q ) - wantDeg ) / adBudget,
		    1.0, Fmt( "%g deg", wantDeg ).c_str() );

		// Invert, conjugate, normalize.
		Quaternion inv, id;
		QuaternionInvert( p, inv );
		QuaternionMult( p, inv, id );
		c.Sample( "quat.invert", 1.0 - std::fabs( id.w ), 2e-6, "" );
		Quaternion un( p.x * 3.0f, p.y * 3.0f, p.z * 3.0f, p.w * 3.0f );
		float len = QuaternionNormalize( un );
		c.Sample( "quat.normalize.length", std::fabs( len - 3.0 ), 2e-6 * 3, "" );
		c.Sample( "quat.normalize", 1.0 - QuaternionDotProduct( un, p ), 2e-6, "" );
	}
	Quaternion zero( 0, 0, 0, 0 );
	c.Check(
	    QuaternionNormalize( zero ) == 0.0f && zero.x == 0 && zero.w == 0, "quat.normalize.zero" );

	// Slerp endpoints and identical inputs.
	for ( int i = 0; i < 500; ++i )
	{
		Quaternion p = rng.UnitQuat(), out;
		QuaternionSlerp( p, p, rng.Float( 0, 1 ), out );
		c.Sample( "quat.slerp.identical", 1.0 - QuaternionDotProduct( p, out ), 2e-6, "" );
	}

	ExpectRejected( c, "quat.slerp.rejects-nlerp",
	    []( Checks &s )
	    {
		    CheckSlerp( s, BadSlerpNlerp, 400 );
	    } );
	ExpectRejected( c, "quat.slerp.rejects-long-arc",
	    []( Checks &s )
	    {
		    CheckSlerp( s, BadSlerpNoAlign, 400 );
	    } );
	ExpectRejected( c, "quat.matrix.rejects-transposed",
	    []( Checks &s )
	    {
		    CheckQuaternionMatrix( s, BadQuatMatrixTransposed, 200 );
	    } );
}

} // namespace

void RunTransformSection( Checks &c )
{
	RunAngles( c );
	RunMatrices( c );
	RunQuaternions( c );
}

} // namespace mathconf
