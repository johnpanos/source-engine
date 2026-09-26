//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyroscopic torque clauses of the VPhysics provider conformance
//          suite (RFC 0004 / RFC 0005 Q-PHYSICS).
//
//          A rigid body whose inertia differs about its axes feels the
//          gyroscopic torque -w x (I w). An integrator that drops it keeps a
//          torque-free body's world angular velocity constant, so angular
//          momentum is not conserved, a spinning plate on a pivot falls
//          instead of precessing, and long slender bodies spin on without
//          tumbling. Every scene measures the body's rotation from its
//          reported orientation, so it judges what the provider does, not what
//          it reports as its velocity, and checks an analytic property of
//          rigid-body rotation:
//
//            free       torque-free body: world angular momentum constant,
//                       world angular velocity not constant;
//            flip       spin about the intermediate principal axis flips over
//                       (the Dzhanibekov effect); spin about the major axis
//                       stays put;
//            gyroscope  a spinning square plate on a ballsocket pivot, axle
//                       level, precesses about the vertical at m g r / (I3 s)
//                       instead of falling;
//            t-handle   Box3D's "Gyroscopic Torque" sample (Dzhanibekov's
//                       wing nut: a bar with a cylindrical handle, spun about
//                       the handle in zero gravity) flips over and back with
//                       the period of Euler's equations, integrated
//                       independently from the same state.
//
//          The "gyro-off" fault holds each body's world angular velocity fixed
//          across every tick, as an integrator without the gyroscopic term
//          does for a torque-free body.
//
//=============================================================================//
#include "vphysics_conformance.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vphysics/constraints.h"
#include "vphysics/performance.h"
#include "tier1/utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const float kMetersPerInch = 0.0254f;

// A world with no gravity for the torque-free scenes, or the suite's
// gravity for the gyroscope.
struct GyroWorld_t
{
	IPhysicsEnvironment *pEnv;
	CPhysCollide *pAnchorCollide;
	IPhysicsObject *pAnchor;
	int material;
};

bool CreateGyroWorld( GyroWorld_t &world, float gravity )
{
	memset( &world, 0, sizeof( world ) );
	world.pEnv = CreateSuiteEnvironment();
	if ( !world.pEnv )
		return false;
	physics_performanceparams_t perf;
	perf.Defaults();
	world.pEnv->SetPerformanceSettings( &perf );
	world.pEnv->SetSimulationTimestep( kTick );
	world.pEnv->SetGravity( Vector( 0, 0, -gravity ) );
	world.material = s_pProps->GetSurfaceIndex( "metal" );
	if ( world.material < 0 )
		world.material = 0;
	// A small static anchor the gyroscope's pivot attaches to.
	world.pAnchorCollide = s_pCollision->BBoxToCollide( Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );
	objectparams_t params = DefaultParams( 1.0f, NULL );
	world.pAnchor = world.pEnv->CreatePolyObjectStatic(
	    world.pAnchorCollide, world.material, Vector( 0, 0, -1000 ), vec3_angle, &params );
	return world.pAnchor != NULL;
}

void DestroyGyroWorld( GyroWorld_t &world )
{
	if ( !world.pEnv )
		return;
	s_pPhysics->DestroyEnvironment( world.pEnv );
	s_pCollision->DestroyCollide( world.pAnchorCollide );
	memset( &world, 0, sizeof( world ) );
}

// A box body with no damping or drag, whose rotation only its inertia
// governs.
struct GyroBody_t
{
	CPhysCollide *pCollide;
	IPhysicsObject *pObject;
	Vector analytic; // solid-box principal inertia, kg*m^2 along local x, y, z
	Vector inertia;  // the provider's principal inertia, which the oracles use
	float mass;
};

bool CreateGyroBody( GyroWorld_t &world, GyroBody_t &body, const Vector &halfExtents, float mass,
    const Vector &position, const QAngle &angles )
{
	memset( &body, 0, sizeof( body ) );
	body.mass = mass;
	body.pCollide = s_pCollision->BBoxToCollide( -halfExtents, halfExtents );
	objectparams_t params = DefaultParams( mass, NULL );
	params.damping = 0.0f;
	params.rotdamping = 0.0f;
	params.dragCoefficient = 0.0f;
	body.pObject =
	    world.pEnv->CreatePolyObject( body.pCollide, world.material, position, angles, &params );
	if ( !body.pObject )
		return false;
	body.pObject->EnableMotion( true );
	body.pObject->EnableDrag( false );
	body.pObject->Wake();
	Vector size = halfExtents * 2.0f * kMetersPerInch;
	body.analytic.Init( mass * ( size.y * size.y + size.z * size.z ) / 12.0f,
	    mass * ( size.x * size.x + size.z * size.z ) / 12.0f,
	    mass * ( size.x * size.x + size.y * size.y ) / 12.0f );
	body.inertia = body.pObject->GetInertia();
	return true;
}

void DestroyGyroBody( GyroWorld_t &world, GyroBody_t &body )
{
	if ( body.pObject )
		world.pEnv->DestroyObject( body.pObject );
	if ( body.pCollide )
		s_pCollision->DestroyCollide( body.pCollide );
	memset( &body, 0, sizeof( body ) );
}

// Orientation as a rotation matrix (columns are the local axes in world).
struct Rotation_t
{
	Vector axis[3];
};

Rotation_t RotationOf( IPhysicsObject *pObject )
{
	Rotation_t r;
	for ( int i = 0; i < 3; i++ )
		r.axis[i] = AxisOf( pObject, i );
	return r;
}

// World angular velocity (rad/s) that turns a into b over dt: the axis-angle
// of b * a^T.
Vector AngularVelocityBetween( const Rotation_t &a, const Rotation_t &b, float dt )
{
	// m = sum_i b.axis[i] a.axis[i]^T
	float m[3][3];
	for ( int row = 0; row < 3; row++ )
	{
		for ( int col = 0; col < 3; col++ )
		{
			m[row][col] = 0.0f;
			for ( int i = 0; i < 3; i++ )
				m[row][col] += b.axis[i][row] * a.axis[i][col];
		}
	}
	float cosine = clamp( ( m[0][0] + m[1][1] + m[2][2] - 1.0f ) * 0.5f, -1.0f, 1.0f );
	float angle = acosf( cosine );
	Vector axis( m[2][1] - m[1][2], m[0][2] - m[2][0], m[1][0] - m[0][1] );
	float length = axis.Length();
	if ( length < 1e-9f || angle < 1e-7f )
		return vec3_origin;
	return axis * ( angle / ( length * dt ) );
}

// The orientation half way from a to b: a turned by omega * dt / 2.
Rotation_t MidRotation( const Rotation_t &a, const Vector &omega, float dt )
{
	Rotation_t mid = a;
	float angle = omega.Length() * dt * 0.5f;
	if ( angle < 1e-9f )
		return mid;
	Vector k = omega / omega.Length();
	float c = cosf( angle ), s = sinf( angle );
	for ( int i = 0; i < 3; i++ )
	{
		const Vector &v = a.axis[i];
		mid.axis[i] = v * c + CrossProduct( k, v ) * s + k * ( DotProduct( k, v ) * ( 1.0f - c ) );
	}
	return mid;
}

// World angular momentum (kg*m^2/s) for a world angular velocity.
Vector MomentumOf( const Rotation_t &r, const Vector &inertia, const Vector &omega )
{
	Vector momentum( 0, 0, 0 );
	for ( int i = 0; i < 3; i++ )
		momentum += r.axis[i] * ( inertia[i] * DotProduct( r.axis[i], omega ) );
	return momentum;
}

float EnergyOf( const Rotation_t &r, const Vector &inertia, const Vector &omega )
{
	float energy = 0.0f;
	for ( int i = 0; i < 3; i++ )
	{
		float w = DotProduct( r.axis[i], omega );
		energy += 0.5f * inertia[i] * w * w;
	}
	return energy;
}

float AngleBetweenDegrees( const Vector &a, const Vector &b )
{
	float la = a.Length();
	float lb = b.Length();
	if ( la < 1e-9f || lb < 1e-9f )
		return 180.0f;
	return RAD2DEG( acosf( clamp( DotProduct( a, b ) / ( la * lb ), -1.0f, 1.0f ) ) );
}

void SetWorldAngularVelocity( IPhysicsObject *pObject, const Vector &omegaWorld )
{
	Vector local;
	pObject->WorldToLocalVector( &local, omegaWorld );
	AngularImpulse degrees = local * ( 180.0f / M_PI_F );
	pObject->SetVelocity( NULL, &degrees );
}

// One tick. Under "gyro-off" the body's world angular velocity is held at its
// pre-step value.
void GyroTick( IPhysicsEnvironment *pEnv, IPhysicsObject *pObject )
{
	Vector held( 0, 0, 0 );
	if ( FaultIs( "gyro-off" ) )
	{
		AngularImpulse local = AngularVelocityOf( pObject );
		pObject->LocalToWorldVector( &held, local * ( M_PI_F / 180.0f ) );
	}
	Step( pEnv, kTick );
	if ( FaultIs( "gyro-off" ) )
		SetWorldAngularVelocity( pObject, held );
}

void Obs1( const char *pKey, const char *pTol, float value )
{
	ObsFloats( pKey, pTol, 1, &value );
}

//-----------------------------------------------------------------------------
// The provider's principal inertia for the box lies along the box's local
// axes, ordered as a solid box's (x < y < z) and near it. The collision
// builder's inertia is not the exact solid-box value (legacy .phy inertia is
// preserved), so the other scenes judge momentum and precession with the
// provider's own inertia.
//-----------------------------------------------------------------------------
void TestBoxInertia()
{
	GyroWorld_t world;
	if ( !Check( TIER_GAMEPLAY, "gyro.world", CreateGyroWorld( world, 0.0f ) ) )
		return;
	GyroBody_t body;
	CreateGyroBody( world, body, Vector( 24, 12, 4 ), 20.0f, Vector( 0, 0, 0 ), vec3_angle );
	bool ok =
	    body.pObject != NULL && body.inertia.x < body.inertia.y && body.inertia.y < body.inertia.z;
	for ( int i = 0; i < 3; i++ )
		ok &= fabsf( body.inertia[i] - body.analytic[i] ) <= 0.25f * body.analytic[i];
	Check( TIER_GAMEPLAY, "gyro.box-inertia", ok,
	    "reported (%.4f %.4f %.4f) solid box (%.4f %.4f %.4f)", body.inertia.x, body.inertia.y,
	    body.inertia.z, body.analytic.x, body.analytic.y, body.analytic.z );
	ObsFloats( "gyro.box-inertia", "r0.01", 3, body.inertia.Base() );
	DestroyGyroBody( world, body );
	DestroyGyroWorld( world );
}

//-----------------------------------------------------------------------------
// Torque-free tumble: a 48x24x8 box (all three principal inertias distinct)
// is set spinning about an axis that is not principal, with gravity off.
// Angular momentum is conserved in world space, and the angular velocity
// wanders around it (polhode motion). Kinetic energy is conserved too; an
// integrator may lose some of it (Box3D's implicit gyroscopic solve damps a
// little), but must not gain it: IVP's explicit substeps do, and it is a
// recorded reference deficiency (tools/quality/physics_conformance.py).
//-----------------------------------------------------------------------------
void TestFreeTumble()
{
	GyroWorld_t world;
	if ( !CreateGyroWorld( world, 0.0f ) )
		return;
	GyroBody_t body;
	CreateGyroBody( world, body, Vector( 24, 12, 4 ), 20.0f, Vector( 0, 0, 0 ), vec3_angle );
	IPhysicsObject *pBox = body.pObject;
	if ( !pBox )
	{
		DestroyGyroWorld( world );
		return;
	}
	// 1 rev/s about each local axis: |w| = 10.9 rad/s, below the 3600 deg/s cap.
	AngularImpulse spin( 360.0f, 360.0f, 360.0f );
	pBox->SetVelocity( NULL, &spin );
	// IVP's first step after SetVelocity turns the body through two ticks of
	// rotation; start measuring from the state after it.
	GyroTick( world.pEnv, pBox );

	const int kTicks = 200; // 3 seconds
	Rotation_t previous = RotationOf( pBox );
	Vector momentum0, omega0;
	float energy0 = 0.0f;
	float maxMomentumAngle = 0.0f;
	float minMomentumRatio = 1.0f, maxMomentumRatio = 1.0f;
	float maxEnergyRatio = 0.0f, endEnergyRatio = 0.0f;
	float maxOmegaAngle = 0.0f;
	for ( int i = 0; i < kTicks; i++ )
	{
		GyroTick( world.pEnv, pBox );
		Rotation_t current = RotationOf( pBox );
		Vector omega = AngularVelocityBetween( previous, current, kTick );
		// The finite difference is the mid-step angular velocity; judge
		// momentum and energy with the mid-step orientation.
		Rotation_t mid = MidRotation( previous, omega, kTick );
		Vector momentum = MomentumOf( mid, body.inertia, omega );
		float energy = EnergyOf( mid, body.inertia, omega );
		if ( i == 0 )
		{
			momentum0 = momentum;
			omega0 = omega;
			energy0 = energy;
		}
		else
		{
			maxMomentumAngle = MAX( maxMomentumAngle, AngleBetweenDegrees( momentum, momentum0 ) );
			float ratio = momentum.Length() / MAX( momentum0.Length(), 1e-9f );
			minMomentumRatio = MIN( minMomentumRatio, ratio );
			maxMomentumRatio = MAX( maxMomentumRatio, ratio );
			endEnergyRatio = energy / MAX( energy0, 1e-9f );
			maxEnergyRatio = MAX( maxEnergyRatio, endEnergyRatio );
			maxOmegaAngle = MAX( maxOmegaAngle, AngleBetweenDegrees( omega, omega0 ) );
		}
		if ( getenv( "GYRO_TRACE" ) )
		{
			AngularImpulse reported = AngularVelocityOf( pBox );
			printf( "GYRO free %d |L| %.4f E %.4f w %.3f %.3f %.3f reported %.2f %.2f %.2f\n", i,
			    momentum.Length(), energy, omega.x, omega.y, omega.z, reported.x, reported.y,
			    reported.z );
		}
		previous = current;
	}
	Check( TIER_GAMEPLAY, "gyro.free-momentum-direction", maxMomentumAngle < 5.0f,
	    "max angle %.3f deg", maxMomentumAngle );
	Check( TIER_GAMEPLAY, "gyro.free-momentum-magnitude",
	    minMomentumRatio > 0.8f && maxMomentumRatio < 1.03f, "|L|/|L0| %.4f..%.4f",
	    minMomentumRatio, maxMomentumRatio );
	Check( TIER_GAMEPLAY, "gyro.free-energy-not-gained", maxEnergyRatio < 1.03f,
	    "max E/E0 %.4f end %.4f", maxEnergyRatio, endEnergyRatio );
	Check( TIER_GAMEPLAY, "gyro.free-velocity-wanders", maxOmegaAngle > 20.0f,
	    "max angle(w, w0) %.3f deg", maxOmegaAngle );
	Obs1( "gyro.free.max-momentum-angle", "a5", maxMomentumAngle );
	Obs1( "gyro.free.max-velocity-angle", "a25", maxOmegaAngle );
	DestroyGyroBody( world, body );
	DestroyGyroWorld( world );
}

//-----------------------------------------------------------------------------
// Intermediate axis theorem: the same box spun about its intermediate axis
// (local y) with a 1% perturbation flips that axis over within a few
// seconds; spun about its major axis (local z) it stays aligned.
//-----------------------------------------------------------------------------
struct FlipResult_t
{
	float minDot;      // min over time of dot(axis, axis at start)
	float flipSeconds; // first time the axis points backwards (-1: never)
};

FlipResult_t SpinAbout( int axis, float seconds )
{
	FlipResult_t result = { 1.0f, -1.0f };
	GyroWorld_t world;
	if ( !CreateGyroWorld( world, 0.0f ) )
		return result;
	GyroBody_t body;
	CreateGyroBody( world, body, Vector( 24, 12, 4 ), 20.0f, Vector( 0, 0, 0 ), vec3_angle );
	IPhysicsObject *pBox = body.pObject;
	if ( pBox )
	{
		AngularImpulse spin( 3.6f, 3.6f, 3.6f );
		spin[axis] = 360.0f;
		pBox->SetVelocity( NULL, &spin );
		Vector start = AxisOf( pBox, axis );
		int ticks = (int)( seconds / kTick + 0.5f );
		for ( int i = 0; i < ticks; i++ )
		{
			GyroTick( world.pEnv, pBox );
			float dot = DotProduct( AxisOf( pBox, axis ), start );
			result.minDot = MIN( result.minDot, dot );
			if ( result.flipSeconds < 0.0f && dot < -0.5f )
				result.flipSeconds = ( i + 1 ) * kTick;
		}
		DestroyGyroBody( world, body );
	}
	DestroyGyroWorld( world );
	return result;
}

void TestIntermediateAxis()
{
	FlipResult_t intermediate = SpinAbout( 1, 6.0f );
	FlipResult_t major = SpinAbout( 2, 6.0f );
	Check( TIER_GAMEPLAY, "gyro.intermediate-axis-flips", intermediate.flipSeconds > 0.0f,
	    "flip %.3f s min dot %.3f", intermediate.flipSeconds, intermediate.minDot );
	Check( TIER_GAMEPLAY, "gyro.major-axis-stable", major.minDot > 0.95f, "min dot %.4f",
	    major.minDot );
	Obs1( "gyro.flip.seconds", "a1.5", intermediate.flipSeconds );
}

//-----------------------------------------------------------------------------
// Gyroscope: a 48x48x4 plate (symmetry axis local z) hangs from a ballsocket
// on its axle, r = 12 in from its center, with the axle level along world x.
// Spun at s about the axle and released, it precesses about the vertical at
// Omega = m g r / (I3 s) with a small nutation instead of swinging down.
// Without gyroscopic torque the plate falls and the spin smears across axes.
//-----------------------------------------------------------------------------
void TestGyroscope()
{
	const float kGravity = 600.0f;      // in/s^2, the suite's gravity
	const float kArm = 12.0f;           // pivot to center, inches
	const float kSpinDegrees = 2400.0f; // about the axle
	const float kSeconds = 4.0f;
	const Vector kPivot( 0, 0, 200 );

	GyroWorld_t world;
	if ( !CreateGyroWorld( world, kGravity ) )
		return;
	GyroBody_t body;
	// Pitch 90 turns local z to world +x: the axle is level.
	CreateGyroBody( world, body, Vector( 24, 24, 2 ), 20.0f, kPivot + Vector( kArm, 0, 0 ),
	    QAngle( 90, 0, 0 ) );
	IPhysicsObject *pPlate = body.pObject;
	if ( !pPlate )
	{
		DestroyGyroWorld( world );
		return;
	}
	constraint_ballsocketparams_t ballsocket;
	ballsocket.Defaults();
	ballsocket.InitWithCurrentObjectState( world.pAnchor, pPlate, kPivot );
	IPhysicsConstraint *pPivot =
	    world.pEnv->CreateBallsocketConstraint( world.pAnchor, pPlate, NULL, ballsocket );
	if ( !Check( TIER_GAMEPLAY, "gyro.gyroscope-pivot", pPivot != NULL ) )
	{
		DestroyGyroBody( world, body );
		DestroyGyroWorld( world );
		return;
	}
	AngularImpulse spin( 0, 0, kSpinDegrees );
	pPlate->SetVelocity( NULL, &spin );

	// I3 in kg*in^2 so the rate is in rad/s with g and r in inches.
	float i3 = body.inertia.z / ( kMetersPerInch * kMetersPerInch );
	float expectedRate = body.mass * kGravity * kArm / ( i3 * DEG2RAD( kSpinDegrees ) );

	int ticks = (int)( kSeconds / kTick + 0.5f );
	float maxDip = 0.0f;
	float azimuth = 0.0f;
	float previousAzimuth = 0.0f;
	float maxPivotError = 0.0f;
	for ( int i = 0; i < ticks; i++ )
	{
		GyroTick( world.pEnv, pPlate );
		Vector axle = AxisOf( pPlate, 2 );
		float dip = RAD2DEG( asinf( clamp( -axle.z, -1.0f, 1.0f ) ) );
		maxDip = MAX( maxDip, fabsf( dip ) );
		// Unwrapped azimuth of the axle about world z.
		float a = atan2f( axle.y, axle.x );
		float delta = a - previousAzimuth;
		while ( delta > M_PI_F )
			delta -= 2.0f * M_PI_F;
		while ( delta < -M_PI_F )
			delta += 2.0f * M_PI_F;
		azimuth += delta;
		previousAzimuth = a;
		Vector pivotWorld;
		pPlate->LocalToWorld( &pivotWorld, ballsocket.constraintPosition[1] );
		maxPivotError = MAX( maxPivotError, ( pivotWorld - kPivot ).Length() );
		if ( getenv( "GYRO_TRACE" ) )
		{
			AngularImpulse w = AngularVelocityOf( pPlate );
			printf( "GYRO scope %d dip %.3f azimuth %.3f pivot %.3f w %.1f %.1f %.1f\n", i, dip,
			    RAD2DEG( azimuth ), ( pivotWorld - kPivot ).Length(), w.x, w.y, w.z );
		}
	}
	float measuredRate = azimuth / kSeconds;
	Check( TIER_GAMEPLAY, "gyro.gyroscope-stays-level", maxDip < 10.0f, "max axle dip %.3f deg",
	    maxDip );
	Check( TIER_GAMEPLAY, "gyro.gyroscope-precesses",
	    measuredRate > 0.0f && fabsf( measuredRate - expectedRate ) < 0.25f * expectedRate,
	    "rate %.4f rad/s expected %.4f (%.1f deg in %.1f s, I %.4f %.4f %.4f)", measuredRate,
	    expectedRate, RAD2DEG( azimuth ), kSeconds, body.inertia.x, body.inertia.y,
	    body.inertia.z );
	Check( TIER_GAMEPLAY, "gyro.gyroscope-pivot-held", maxPivotError < 2.0f, "max pivot error %.3f",
	    maxPivotError );
	Obs1( "gyro.gyroscope.rate", "r0.1", measuredRate );
	world.pEnv->DestroyConstraint( pPivot );
	DestroyGyroBody( world, body );
	DestroyGyroWorld( world );
}

//-----------------------------------------------------------------------------
// The Dzhanibekov effect as Box3D's samples app shows it (samples/
// sample_bodies.cpp, GyroscopicTorque): a 2 x 0.1 x 0.2 m bar with a
// 32-sided cylinder handle (radius 0.15 m, 0.6 m tall) standing on it, in
// zero gravity, spun at 10 rad/s about the handle with a 0.01 rad/s nudge on
// the other axes. The handle is the intermediate principal axis, so the
// body flips over and back periodically. The oracle integrates Euler's
// torque-free equations (RK4, 50 substeps per tick) with the provider's own
// principal inertia from the provider's first measured body-frame angular
// velocity, and compares the flips (sign changes of the handle component of
// the angular velocity, with hysteresis) and their period.
//-----------------------------------------------------------------------------
struct FlipTrack_t
{
	float threshold;
	int sign;
	CUtlVector<float> times;

	void Init( float first )
	{
		threshold = 0.5f * fabsf( first );
		sign = first >= 0.0f ? 1 : -1;
		times.RemoveAll();
	}
	void Sample( float value, float time )
	{
		if ( fabsf( value ) < threshold )
			return;
		int current = value >= 0.0f ? 1 : -1;
		if ( current != sign )
		{
			times.AddToTail( time );
			sign = current;
		}
	}
	// Mean time between flips after the first (the first depends on the
	// nudge; the rest are the orbit's half period).
	float MeanInterval() const
	{
		if ( times.Count() < 2 )
			return -1.0f;
		return ( times[times.Count() - 1] - times[0] ) / ( times.Count() - 1 );
	}
};

Vector EulerRate( const Vector &inertia, const Vector &w )
{
	return Vector( ( inertia.y - inertia.z ) * w.y * w.z / inertia.x,
	    ( inertia.z - inertia.x ) * w.z * w.x / inertia.y,
	    ( inertia.x - inertia.y ) * w.x * w.y / inertia.z );
}

Vector EulerStep( const Vector &inertia, const Vector &w, float h )
{
	Vector k1 = EulerRate( inertia, w );
	Vector k2 = EulerRate( inertia, w + k1 * ( 0.5f * h ) );
	Vector k3 = EulerRate( inertia, w + k2 * ( 0.5f * h ) );
	Vector k4 = EulerRate( inertia, w + k3 * h );
	return w + ( k1 + k2 * 2.0f + k3 * 2.0f + k4 ) * ( h / 6.0f );
}

CPhysCollide *THandleCollide()
{
	const float kInchesPerMeter = 1.0f / kMetersPerInch;
	const int kSides = 32;
	Vector cylinder[2 * kSides];
	Vector *pCylinder[2 * kSides];
	for ( int i = 0; i < kSides; i++ )
	{
		float alpha = 2.0f * M_PI_F * i / kSides;
		float x = 0.15f * cosf( alpha ) * kInchesPerMeter, z = 0.15f * sinf( alpha ) * kInchesPerMeter;
		cylinder[2 * i].Init( x, 0.0f, z );
		cylinder[2 * i + 1].Init( x, 0.6f * kInchesPerMeter, z );
		pCylinder[2 * i] = &cylinder[2 * i];
		pCylinder[2 * i + 1] = &cylinder[2 * i + 1];
	}
	Vector half = Vector( 1.0f, 0.05f, 0.1f ) * kInchesPerMeter;
	Vector bar[8];
	Vector *pBar[8];
	for ( int i = 0; i < 8; i++ )
	{
		bar[i].Init( ( i & 1 ) ? half.x : -half.x, ( i & 2 ) ? half.y : -half.y,
		    ( i & 4 ) ? half.z : -half.z );
		pBar[i] = &bar[i];
	}
	CPhysConvex *pConvexes[2] = { s_pCollision->ConvexFromVerts( pCylinder, 2 * kSides ),
		s_pCollision->ConvexFromVerts( pBar, 8 ) };
	if ( !pConvexes[0] || !pConvexes[1] )
		return NULL;
	return s_pCollision->ConvertConvexToCollide( pConvexes, 2 );
}

void TestDzhanibekov()
{
	// Long enough for four flips at the legacy model's 5 s interval.
	const float kSeconds = 25.0f;
	GyroWorld_t world;
	if ( !CreateGyroWorld( world, 0.0f ) )
		return;
	CPhysCollide *pCollide = THandleCollide();
	IPhysicsObject *pBody = NULL;
	if ( pCollide )
	{
		objectparams_t params = DefaultParams( 5.0f, NULL );
		params.damping = 0.0f;
		params.rotdamping = 0.0f;
		params.dragCoefficient = 0.0f;
		pBody = world.pEnv->CreatePolyObject( pCollide, world.material, vec3_origin, vec3_angle, &params );
	}
	if ( !Check( TIER_GAMEPLAY, "gyro.t-handle-body", pBody != NULL ) )
	{
		if ( pCollide )
			s_pCollision->DestroyCollide( pCollide );
		DestroyGyroWorld( world );
		return;
	}
	pBody->EnableMotion( true );
	pBody->EnableDrag( false );
	pBody->Wake();
	Vector inertia = pBody->GetInertia();
	bool intermediate = ( inertia.y > MIN( inertia.x, inertia.z ) && inertia.y < MAX( inertia.x, inertia.z ) );
	Check( TIER_GAMEPLAY, "gyro.t-handle-intermediate", intermediate, "inertia (%g %g %g)", inertia.x, inertia.y,
	    inertia.z );

	// The sample's world (0.01, 0.01, 10) rad/s on a body turned -90 degrees
	// about x: (0.01, -10, 0.01) in the body frame.
	AngularImpulse spin = AngularImpulse( 0.01f, -10.0f, 0.01f ) * ( 180.0f / M_PI_F );
	pBody->SetVelocity( NULL, &spin );
	GyroTick( world.pEnv, pBody ); // see TestFreeTumble: IVP's first step after SetVelocity

	int ticks = (int)( kSeconds / kTick + 0.5f );
	Rotation_t previous = RotationOf( pBody );
	FlipTrack_t measured, reference;
	Vector referenceW( 0, 0, 0 ), momentum0( 0, 0, 0 );
	float maxMomentumAngle = 0.0f, minRatio = 1.0f, maxRatio = 1.0f;
	for ( int i = 0; i < ticks; i++ )
	{
		GyroTick( world.pEnv, pBody );
		Rotation_t current = RotationOf( pBody );
		Vector omega = AngularVelocityBetween( previous, current, kTick );
		Rotation_t mid = MidRotation( previous, omega, kTick );
		Vector body( DotProduct( mid.axis[0], omega ), DotProduct( mid.axis[1], omega ),
		    DotProduct( mid.axis[2], omega ) );
		Vector momentum = MomentumOf( mid, inertia, omega );
		float time = ( i + 0.5f ) * kTick;
		if ( i == 0 )
		{
			// The oracle starts from the provider's first measured state.
			referenceW = body;
			momentum0 = momentum;
			measured.Init( body.y );
			reference.Init( body.y );
		}
		else
		{
			const int kSubsteps = 50;
			for ( int k = 0; k < kSubsteps; k++ )
				referenceW = EulerStep( inertia, referenceW, kTick / kSubsteps );
			measured.Sample( body.y, time );
			reference.Sample( referenceW.y, time );
			maxMomentumAngle = MAX( maxMomentumAngle, AngleBetweenDegrees( momentum, momentum0 ) );
			float ratio = momentum.Length() / MAX( momentum0.Length(), 1e-9f );
			minRatio = MIN( minRatio, ratio );
			maxRatio = MAX( maxRatio, ratio );
		}
		if ( getenv( "GYRO_TRACE" ) )
		{
			printf( "GYRO thandle %d w %.3f %.3f %.3f ref %.3f %.3f %.3f |L| %.5f\n", i, body.x, body.y,
			    body.z, referenceW.x, referenceW.y, referenceW.z, momentum.Length() );
		}
		previous = current;
	}
	float period = measured.MeanInterval(), expected = reference.MeanInterval();
	Check( TIER_GAMEPLAY, "gyro.t-handle-flips",
	    measured.times.Count() >= 4 && abs( measured.times.Count() - reference.times.Count() ) <= 1,
	    "%d flips in %.0f s, Euler's equations %d (first %.2f s, reference %.2f s)", measured.times.Count(),
	    kSeconds, reference.times.Count(), measured.times.Count() ? measured.times[0] : -1.0f,
	    reference.times.Count() ? reference.times[0] : -1.0f );
	Check( TIER_GAMEPLAY, "gyro.t-handle-period",
	    period > 0.0f && expected > 0.0f && fabsf( period - expected ) < 0.15f * expected,
	    "flip interval %.3f s, Euler's equations %.3f s", period, expected );
	Check( TIER_GAMEPLAY, "gyro.t-handle-momentum", maxMomentumAngle < 5.0f && minRatio > 0.8f && maxRatio < 1.03f,
	    "max angle %.3f deg |L|/|L0| %.4f..%.4f", maxMomentumAngle, minRatio, maxRatio );
	Obs1( "gyro.t-handle.flip-interval", "r0.15", period );
	world.pEnv->DestroyObject( pBody );
	s_pCollision->DestroyCollide( pCollide );
	DestroyGyroWorld( world );
}
}

void TestGyroscopic()
{
	TestBoxInertia();
	TestFreeTumble();
	TestIntermediateAxis();
	TestGyroscope();
	TestDzhanibekov();
}
