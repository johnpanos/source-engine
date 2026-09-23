//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Vehicle controllers on Box3D. See physics_vehicle.h.
//
//          The controller is a port of vphysics/physics_vehicle.cpp. The car
//          systems reproduce IVP's actuators as impulses applied before each
//          Box3D step (IVP applies them inside its step as controllers):
//
//          real wheels  IVP_Car_System_Real_Wheels + IVP_Constraint_Solver_Car:
//                       wheel objects on Box3D wheel joints (free along the
//                       body's vertical, spin about the axle, steering locked
//                       by equal limits); suspension, wheel torque,
//                       stabilizer, down/extra-gravity/powerslide/booster
//                       forces as IVP_Actuator_* compute them.
//          airboat      CPhysics_Airboat (vphysics/physics_airboat.cpp),
//                       ported line for line on an IVP-frame view of the body.
//
//=============================================================================//
#include "physics_vehicle.h"

#include <math.h>
#include <string.h>

#include "box3d/box3d.h"
#include "box3d_convert.h"
#include "bspflags.h"
#include "cmodel.h"
#include "gametrace.h"
#include "mathlib/mathlib.h"
#include "physics_controllers.h"
#include "physics_environment.h"
#include "physics_material.h"
#include "physics_object.h"
#include "vphysics/friction.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const float kMetersPerInch = 0.0254f;
const float kPi = 3.14159265358979f;

inline float ConvertDistanceToHL( float meters ) { return meters / kMetersPerInch; }
inline float ConvertDistanceToIVP( float inches ) { return inches * kMetersPerInch; }

#define THROTTLE_OPPOSING_FORCE_EPSILON		5.0f
#define MPH_TO_METERSPERSECOND	0.44707f
#define METERSPERSECOND_TO_MPH	( 1.0f / MPH_TO_METERSPERSECOND )
#define MPH_TO_GAMEVEL( x )	( ConvertDistanceToHL( ( x ) * MPH_TO_METERSPERSECOND ) )
#define GAMEVEL_TO_MPH( x )	( ConvertDistanceToIVP( x ) * METERSPERSECOND_TO_MPH )
#define FVEHICLE_THROTTLE_STOPPED		0x00000001
#define FVEHICLE_HANDBRAKE_ON			0x00000002

// IVP axes: x = Source x, y = -Source z (down), z = Source y (forward).
inline Vector HLToIvpDir( const Vector &v ) { return Vector( v.x, -v.z, v.y ); }
inline Vector IvpToHLDir( const Vector &v ) { return Vector( v.x, v.z, -v.y ); }
inline Vector HLToIvpPos( const Vector &v ) { return HLToIvpDir( v ) * kMetersPerInch; }
inline Vector IvpToHLPos( const Vector &v ) { return IvpToHLDir( v ) / kMetersPerInch; }

//-----------------------------------------------------------------------------
// A body seen as an IVP core: origin at the mass center, the object's axes,
// meters and newtons. Pushes are applied to the Box3D body immediately.
//-----------------------------------------------------------------------------
class CIvpCore
{
public:
	explicit CIvpCore( CPhysicsObjectBox3D *pObject ) : m_pObject( pObject )
	{
		m_body = pObject->GetBody();
		m_rotation = b3Body_GetRotation( m_body );
		m_centerHL = FromB3( b3Body_GetWorldCenter( m_body ) );
	}

	bool IsSimulated() const { return m_pObject->IsMoveable(); }
	float Mass() const { return m_pObject->GetMass(); }
	Vector CoreToWorldDir( const Vector &cs ) const { return HLToIvpDir( FromB3( b3RotateVector( m_rotation, ToB3( IvpToHLDir( cs ) ) ) ) ); }
	Vector WorldToCoreDir( const Vector &ws ) const { return HLToIvpDir( FromB3( b3InvRotateVector( m_rotation, ToB3( IvpToHLDir( ws ) ) ) ) ); }
	Vector Position() const { return HLToIvpPos( m_centerHL ); }
	Vector CoreToWorldPos( const Vector &cs ) const { return Position() + CoreToWorldDir( cs ); }
	// Column index of the core-to-world rotation (IVP get_col).
	Vector Axis( int index ) const
	{
		Vector axis( 0, 0, 0 );
		axis[index] = 1.0f;
		return CoreToWorldDir( axis );
	}
	Vector Speed() const { return HLToIvpDir( FromB3( b3Body_GetLinearVelocity( m_body ) ) ) * kMetersPerInch; }
	// Angular velocity in core axes, rad/s.
	Vector RotSpeed() const { return WorldToCoreDir( HLToIvpDir( FromB3( b3Body_GetAngularVelocity( m_body ) ) ) ); }
	Vector SurfaceSpeed( const Vector &pointWS ) const
	{
		return HLToIvpDir( FromB3( b3Body_GetWorldPointVelocity( m_body, ToB3( IvpToHLPos( pointWS ) ) ) ) ) * kMetersPerInch;
	}
	// Linear impulse (N s) at a world point.
	void PushWS( const Vector &pointWS, const Vector &impulseWS ) const
	{
		if ( IsSimulated() )
			b3Body_ApplyLinearImpulse( m_body, ToB3( IvpToHLDir( impulseWS ) / kMetersPerInch ), ToB3( IvpToHLPos( pointWS ) ), true );
	}
	void CenterPushWS( const Vector &impulseWS ) const
	{
		if ( IsSimulated() )
			b3Body_ApplyLinearImpulseToCenter( m_body, ToB3( IvpToHLDir( impulseWS ) / kMetersPerInch ), true );
	}
	// Angular impulse (kg m^2/s) in core axes.
	void RotPushCS( const Vector &angularCS ) const
	{
		if ( IsSimulated() )
			b3Body_ApplyAngularImpulse( m_body, ToB3( IvpToHLDir( CoreToWorldDir( angularCS ) ) * kInertiaToBox3D ), true );
	}

private:
	CPhysicsObjectBox3D *m_pObject;
	b3BodyId m_body;
	b3Quat m_rotation;
	Vector m_centerHL;
};

// IVP_Car_System::calc_ackerman_angle
float CalcAckermanAngle( float alpha, float dx, float dz )
{
	float a = fabsf( alpha );
	if ( a < 0.001f )
		return alpha;
	double tanAlpha = tan( a );
	double h = ( dz * tanAlpha ) / ( dx * tanAlpha + dz );
	double beta = atan( h );
	return (float)( beta * ( alpha < 0.0f ? -1.0 : 1.0 ) );
}

// What the controller builds the car system from (IVP_Template_Car_System).
struct CarTemplate_t
{
	int wheelCount;
	int axleCount;
	CPhysicsObjectBox3D *pBody;
	CPhysicsObjectBox3D *pWheels[VEHICLE_MAX_WHEEL_COUNT];
	Vector wheelPositionLocal[VEHICLE_MAX_WHEEL_COUNT];		// Source units, body space
	Vector tracePositionLocal[VEHICLE_MAX_WHEEL_COUNT];
	float wheelRadius[VEHICLE_MAX_WHEEL_COUNT];				// meters
	float frictionOfWheel[VEHICLE_MAX_WHEEL_COUNT];
	float springConstant[VEHICLE_MAX_WHEEL_COUNT];			// N/m
	float springDamping[VEHICLE_MAX_WHEEL_COUNT];
	float springDampingCompression[VEHICLE_MAX_WHEEL_COUNT];
	float maxBodyForce[VEHICLE_MAX_WHEEL_COUNT];			// N
	float springPreTension[VEHICLE_MAX_WHEEL_COUNT];		// meters
	float stabilizerConstant[VEHICLE_MAX_AXLE_COUNT];
	float wheelMaxRotationSpeed[VEHICLE_MAX_AXLE_COUNT];			// rad/s
	float bodyCounterTorqueFactor;
	float bodyDownForceVerticalOffset;						// meters (IVP y)
	float extraGravityForce;								// N
};
}

//-----------------------------------------------------------------------------
// Car system interface (IVP_Car_System's calls, IVP units).
//-----------------------------------------------------------------------------
class CCarSystemBox3D
{
public:
	virtual ~CCarSystemBox3D() {}
	virtual void Simulate( float dt ) = 0;
	virtual void DoSteering( float angle, bool analog ) = 0;
	virtual void ChangeSpringLength( int wheel, float length ) = 0;
	virtual void ChangeWheelTorque( int wheel, float torque ) = 0;
	virtual void UpdateBodyCountertorque() {}
	virtual void UpdateThrottle( float throttle ) {}
	virtual void ChangeBodyDownforce( float force ) = 0;
	virtual void FixWheel( int wheel, bool stop ) = 0;
	virtual double GetBodySpeed() = 0;
	virtual double GetWheelAngularVelocity( int wheel ) = 0;
	virtual void UpdateWheelPositions() {}
	virtual void SetPowerslide( float frontAccel, float rearAccel ) {}
	virtual void ActivateBooster( float thrust, float duration, float delay ) {}
	virtual void UpdateBooster( float dt ) {}
	virtual float GetBoosterDelay() { return 0.0f; }
	virtual float GetBoosterTimeToGo() { return 0.0f; }
	virtual void SetWheelFriction( int wheel, float friction ) {}
	virtual void GetDebugData( vehicle_debugcarsystem_t &debug ) {}
	virtual void ObjectDestroyed( IPhysicsObject *pObject ) {}
};

//-----------------------------------------------------------------------------
// IVP_Car_System_Real_Wheels
//-----------------------------------------------------------------------------
class CCarSystemRealWheels : public CCarSystemBox3D
{
public:
	explicit CCarSystemRealWheels( const CarTemplate_t &templ );
	virtual ~CCarSystemRealWheels();

	virtual void Simulate( float dt ) override;
	virtual void DoSteering( float angle, bool analog ) override;
	virtual void ChangeSpringLength( int wheel, float length ) override { m_wheels[wheel].springLength = length; }
	virtual void ChangeWheelTorque( int wheel, float torque ) override { m_wheels[wheel].torque = torque; }
	virtual void UpdateBodyCountertorque() override;
	virtual void ChangeBodyDownforce( float force ) override { m_downForce = force; }
	virtual void FixWheel( int wheel, bool stop ) override;
	virtual double GetBodySpeed() override;
	virtual double GetWheelAngularVelocity( int wheel ) override { return m_wheels[wheel].rotSpeedOut; }
	virtual void SetPowerslide( float frontAccel, float rearAccel ) override;
	virtual void ActivateBooster( float thrust, float duration, float delay ) override;
	virtual void UpdateBooster( float dt ) override;
	virtual float GetBoosterDelay() override { return m_boosterUntilReady; }
	virtual float GetBoosterTimeToGo() override { return m_boosterToGo; }
	virtual void ObjectDestroyed( IPhysicsObject *pObject ) override;

private:
	struct Wheel_t
	{
		CPhysicsObjectBox3D *pObject;
		b3JointId joint;
		Vector hardPointLocal;	// Source units, body space
		float springConstant;
		float springDamping;
		float springDampingCompression;
		float maxBodyForce;
		float springLength;		// meters, from the anchor 500 m above
		float torque;			// N m about the axle
		float maxRotationSpeed;
		float rotSpeedOut;
		bool fixed;
	};

	void SteerWheel( int wheel, float angle );
	Vector BodyAnchor( int wheel ) const;
	Vector AxleAxis( const Wheel_t &wheel ) const;
	void ApplyTorque( CPhysicsObjectBox3D *pObject, const Vector &axisHL, float torque, float maxSpeed, float dt, float *pSpeedOut );
	void ApplyForce( const Vector &pointHL, const Vector &directionHL, float force, float dt );

	CPhysicsObjectBox3D *m_pBody;
	Wheel_t m_wheels[VEHICLE_MAX_WHEEL_COUNT];
	int m_wheelCount;
	int m_axleCount;
	float m_stabilizerConstant[2];
	float m_bodyCounterTorqueFactor;
	float m_bodyTorque;
	float m_downForce;
	float m_downForceOffset;
	float m_extraGravityForce;
	float m_powerslideFront;	// on the front anchors (IVP's "car_act_powerslide_front")
	float m_powerslideBack;
	float m_boosterForward;		// N, 0 = off
	float m_boosterUp;
	float m_boosterToGo;
	float m_boosterUntilReady;
	float m_steeringAngle;
	float m_fastTurnFactor;
};

// The spring's body anchor: IVP puts it 500 m above the hard point, so the
// spring is effectively vertical in body space.
static const float kSpringAnchorMeters = 500.0f;

CCarSystemRealWheels::CCarSystemRealWheels( const CarTemplate_t &templ )
{
	m_pBody = templ.pBody;
	m_wheelCount = templ.wheelCount;
	m_axleCount = templ.axleCount;
	m_bodyCounterTorqueFactor = templ.bodyCounterTorqueFactor;
	m_bodyTorque = 0.0f;
	m_downForce = 0.0f;
	m_downForceOffset = templ.bodyDownForceVerticalOffset;
	m_extraGravityForce = templ.extraGravityForce;
	m_powerslideFront = m_powerslideBack = 0.0f;
	m_boosterForward = m_boosterUp = 0.0f;
	m_boosterToGo = m_boosterUntilReady = 0.0f;
	m_fastTurnFactor = 1.0f;
	m_stabilizerConstant[0] = templ.stabilizerConstant[0];
	m_stabilizerConstant[1] = templ.stabilizerConstant[1];

	b3WorldId world = m_pBody->GetEnvironment()->GetWorld();
	b3Quat bodyRotation = b3Body_GetRotation( m_pBody->GetBody() );
	// Joint frame: x = the body's vertical (suspension/steering axis), z =
	// the axle (-x, the direction IVP's wheel torque turns about).
	b3Matrix3 frame;
	frame.cx = b3Vec3{ 0, 0, 1 };
	frame.cy = b3Vec3{ 0, 1, 0 };
	frame.cz = b3Vec3{ -1, 0, 0 };
	b3Quat frameRotation = b3MakeQuatFromMatrix( &frame );
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		Wheel_t &wheel = m_wheels[i];
		memset( &wheel, 0, sizeof( wheel ) );
		wheel.pObject = templ.pWheels[i];
		wheel.hardPointLocal = templ.wheelPositionLocal[i];
		wheel.springConstant = templ.springConstant[i];
		wheel.springDamping = templ.springDamping[i];
		wheel.springDampingCompression = templ.springDampingCompression[i];
		wheel.maxBodyForce = templ.maxBodyForce[i];
		wheel.springLength = kSpringAnchorMeters - templ.springPreTension[i];
		wheel.maxRotationSpeed = templ.wheelMaxRotationSpeed[( i & 2 ) ? 1 : 0];
		if ( !wheel.pObject )
			continue;

		b3WheelJointDef def = b3DefaultWheelJointDef();
		def.base.bodyIdA = m_pBody->GetBody();
		def.base.bodyIdB = wheel.pObject->GetBody();
		def.base.localFrameA.p = ToB3( wheel.hardPointLocal );
		def.base.localFrameA.q = frameRotation;
		// Frame B matches frame A in the wheel's current orientation.
		b3Quat wheelRotation = b3Body_GetRotation( wheel.pObject->GetBody() );
		def.base.localFrameB.p = b3Vec3_zero;
		def.base.localFrameB.q = b3MulQuat( b3Conjugate( wheelRotation ), b3MulQuat( bodyRotation, frameRotation ) );
		def.base.collideConnected = false;
		def.base.userData = this;
		def.enableSuspensionSpring = false;		// IVP's suspension is applied as forces
		def.enableSuspensionLimit = false;
		def.enableSpinMotor = false;
		def.enableSteering = true;
		def.enableSteeringLimit = true;
		def.lowerSteeringLimit = 0.0f;
		def.upperSteeringLimit = 0.0f;
		def.steeringHertz = 0.0f;
		wheel.joint = b3CreateWheelJoint( world, &def );
	}
	m_steeringAngle = -1.0f;	// make sure the next call is not optimized away
	DoSteering( 0.0f, false );
}

CCarSystemRealWheels::~CCarSystemRealWheels()
{
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		if ( b3Joint_IsValid( m_wheels[i].joint ) )
			b3DestroyJoint( m_wheels[i].joint, false );
	}
}

void CCarSystemRealWheels::ObjectDestroyed( IPhysicsObject *pObject )
{
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		if ( m_wheels[i].pObject == pObject )
		{
			if ( b3Joint_IsValid( m_wheels[i].joint ) )
				b3DestroyJoint( m_wheels[i].joint, false );
			m_wheels[i].joint = b3_nullJointId;
			m_wheels[i].pObject = NULL;
		}
	}
}

Vector CCarSystemRealWheels::BodyAnchor( int wheel ) const
{
	Vector local = m_wheels[wheel].hardPointLocal + Vector( 0, 0, ConvertDistanceToHL( kSpringAnchorMeters ) );
	Vector world;
	m_pBody->LocalToWorld( &world, local );
	return world;
}

// The wheel's axle (the wheel's -x), the axis IVP's wheel torque turns about.
Vector CCarSystemRealWheels::AxleAxis( const Wheel_t &wheel ) const
{
	return -FromB3( b3Body_GetWorldVector( wheel.pObject->GetBody(), b3Vec3{ 1, 0, 0 } ) );
}

// IVP_Actuator_Torque: a torque impulse about the axis unless the object
// already spins faster than the limit about it.
void CCarSystemRealWheels::ApplyTorque( CPhysicsObjectBox3D *pObject, const Vector &axisHL, float torque, float maxSpeed,
	float dt, float *pSpeedOut )
{
	if ( pSpeedOut )
		*pSpeedOut = 0.0f;
	if ( torque == 0.0f || !pObject->IsMoveable() )
		return;
	float omega = DotProduct( FromB3( b3Body_GetAngularVelocity( pObject->GetBody() ) ), axisHL );
	if ( pSpeedOut )
		*pSpeedOut = omega;
	if ( fabsf( omega ) > maxSpeed )
		return;
	b3Body_ApplyAngularImpulse( pObject->GetBody(), ToB3( axisHL * ( torque * dt * kInertiaToBox3D ) ), true );
}

// IVP_Actuator_Force pushing only the body.
void CCarSystemRealWheels::ApplyForce( const Vector &pointHL, const Vector &directionHL, float force, float dt )
{
	if ( force == 0.0f || !m_pBody->IsMoveable() )
		return;
	b3Body_ApplyLinearImpulse( m_pBody->GetBody(), ToB3( directionHL * ( force * dt / kMetersPerInch ) ), ToB3( pointHL ), true );
}

void CCarSystemRealWheels::Simulate( float dt )
{
	if ( dt <= 0.0f || m_pBody->IsAsleep() )
		return;
	b3BodyId body = m_pBody->GetBody();
	Vector anchors[VEHICLE_MAX_WHEEL_COUNT], centers[VEHICLE_MAX_WHEEL_COUNT];

	// Suspension (IVP_Actuator_Suspension).
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		Wheel_t &wheel = m_wheels[i];
		if ( !wheel.pObject )
			continue;
		anchors[i] = BodyAnchor( i );
		wheel.pObject->GetPosition( &centers[i], NULL );
		Vector dir = anchors[i] - centers[i];
		float dlen = VectorNormalize( dir ) * kMetersPerInch;
		if ( dlen < 1e-6f )
			continue;
		float force = ( dlen - wheel.springLength ) * wheel.springConstant;
		// Positive when the ends close on each other.
		Vector relative = FromB3( b3Body_GetWorldPointVelocity( wheel.pObject->GetBody(), ToB3( centers[i] ) ) ) -
			FromB3( b3Body_GetWorldPointVelocity( body, ToB3( anchors[i] ) ) );
		float dampSpeed = DotProduct( relative, dir ) * kMetersPerInch;
		float damping = dampSpeed < 0.0f ? wheel.springDamping : wheel.springDampingCompression;
		force -= damping * dampSpeed;
		float bodyForce = clamp( -force, -wheel.maxBodyForce, wheel.maxBodyForce );
		// The body's push acts along the same line through the wheel center.
		if ( wheel.pObject->IsMoveable() )
			b3Body_ApplyLinearImpulse( wheel.pObject->GetBody(), ToB3( dir * ( force * dt / kMetersPerInch ) ), ToB3( centers[i] ), true );
		if ( m_pBody->IsMoveable() )
			b3Body_ApplyLinearImpulse( body, ToB3( dir * ( bodyForce * dt / kMetersPerInch ) ), ToB3( centers[i] ), true );
	}

	// Stabilizers (IVP_Actuator_Stabilizer) on the first two axles.
	if ( m_wheelCount != m_axleCount && m_wheelCount >= 4 )
	{
		for ( int axle = 0; axle < 2; axle++ )
		{
			int a = axle * 2, b = axle * 2 + 1;
			if ( !m_wheels[a].pObject || !m_wheels[b].pObject || m_stabilizerConstant[axle] == 0.0f )
				continue;
			Vector dirs[2] = { anchors[a] - centers[a], anchors[b] - centers[b] };
			float dists[2];
			for ( int k = 0; k < 2; k++ )
				dists[k] = VectorNormalize( dirs[k] ) * kMetersPerInch;
			float impulse = ( dists[1] - dists[0] ) * m_stabilizerConstant[axle] * dt;
			int wheels[2] = { a, b };
			for ( int k = 0; k < 2; k++ )
			{
				Vector push = dirs[k] * ( impulse / kMetersPerInch );
				CPhysicsObjectBox3D *pWheel = m_wheels[wheels[k]].pObject;
				if ( pWheel->IsMoveable() )
					b3Body_ApplyLinearImpulse( pWheel->GetBody(), ToB3( push ), ToB3( centers[wheels[k]] ), true );
				if ( m_pBody->IsMoveable() )
					b3Body_ApplyLinearImpulse( body, ToB3( -push ), ToB3( centers[wheels[k]] ), true );
				impulse = -impulse;
			}
		}
	}

	// Wheel torques and the body's counter torque (IVP_Actuator_Torque).
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		Wheel_t &wheel = m_wheels[i];
		wheel.rotSpeedOut = 0.0f;
		if ( wheel.pObject )
			ApplyTorque( wheel.pObject, AxleAxis( wheel ), wheel.torque, wheel.maxRotationSpeed, dt, &wheel.rotSpeedOut );
	}
	Vector bodyAxle = -FromB3( b3Body_GetWorldVector( body, b3Vec3{ 1, 0, 0 } ) );
	ApplyTorque( m_pBody, bodyAxle, m_bodyTorque, kPi * 100.0f, dt, NULL );

	// Body forces (IVP_Actuator_Force, first object only).
	Vector center = FromB3( b3Body_GetWorldCenter( body ) );
	Vector up;
	m_pBody->GetEnvironment()->GetGravity( &up );
	up = -up;
	VectorNormalize( up );
	Vector bodyUp = FromB3( b3Body_GetWorldVector( body, b3Vec3{ 0, 0, 1 } ) );
	Vector bodyRight = FromB3( b3Body_GetWorldVector( body, b3Vec3{ 1, 0, 0 } ) );
	Vector bodyForward = FromB3( b3Body_GetWorldVector( body, b3Vec3{ 0, 1, 0 } ) );
	// Down force at the mass center offset along the body's IVP y (down).
	ApplyForce( center - bodyUp * ConvertDistanceToHL( m_downForceOffset ), -up, m_downForce, dt );
	ApplyForce( center, -up, m_extraGravityForce, dt );
	// Powerslide: sideways (-x) forces 2 m ahead of and behind the mass
	// center. IVP applies the front value at the back and vice versa.
	float twoMeters = ConvertDistanceToHL( 2.0f );
	ApplyForce( center - bodyRight * ConvertDistanceToHL( 1.0f ) + bodyForward * twoMeters, -bodyRight, m_powerslideFront, dt );
	ApplyForce( center - bodyRight * ConvertDistanceToHL( 1.0f ) - bodyForward * twoMeters, -bodyRight, m_powerslideBack, dt );
	// Booster: forward thrust 1 m ahead of the mass center, and lift.
	if ( m_boosterForward != 0.0f )
	{
		ApplyForce( center + bodyForward * ConvertDistanceToHL( 1.0f ), bodyForward, m_boosterForward, dt );
		ApplyForce( center, -up, m_boosterUp, dt );
	}
}

void CCarSystemRealWheels::UpdateBodyCountertorque()
{
	float counterTorque = 0.0f;
	for ( int i = 0; i < m_wheelCount; i++ )
		counterTorque -= m_wheels[i].torque;
	m_bodyTorque = counterTorque * m_bodyCounterTorqueFactor;
}

// IVP turns the wheel's target frame about the body's IVP y (down); the
// joint steers about the body's up axis.
void CCarSystemRealWheels::SteerWheel( int wheel, float angle )
{
	if ( wheel >= m_wheelCount || !b3Joint_IsValid( m_wheels[wheel].joint ) )
		return;
	b3WheelJoint_SetSteeringLimits( m_wheels[wheel].joint, -angle, -angle );
}

void CCarSystemRealWheels::DoSteering( float angle, bool analog )
{
	if ( m_steeringAngle == angle )
		return;
	float dAlpha = angle - m_steeringAngle;
	float dxFrontWheels = 1.0f, dzAxles = 1.0f;
	if ( m_wheelCount >= 4 )
	{
		// get_orig_front_wheel_distance / get_orig_axles_distance.
		dxFrontWheels = ConvertDistanceToIVP( m_wheels[1].hardPointLocal.x - m_wheels[0].hardPointLocal.x );
		dzAxles = fabsf( ConvertDistanceToIVP( m_wheels[0].hardPointLocal.y - m_wheels[2].hardPointLocal.y ) );
	}
	// Start the body turning (rot_speed_change about IVP y).
	float angularSpin = dAlpha * (float)GetBodySpeed() / dzAxles;
	if ( m_pBody->IsMoveable() )
	{
		Vector spinWorld = FromB3( b3Body_GetWorldVector( m_pBody->GetBody(), b3Vec3{ 0, 0, 1 } ) ) * ( angularSpin * m_fastTurnFactor );
		b3Body_SetAngularVelocity( m_pBody->GetBody(), ToB3( FromB3( b3Body_GetAngularVelocity( m_pBody->GetBody() ) ) + spinWorld ) );
	}
	m_steeringAngle = angle;
	int wheelsPerAxle = m_axleCount ? m_wheelCount / m_axleCount : m_wheelCount;
	for ( int i = 0; i < wheelsPerAxle; i++ )
	{
		if ( ( i & 1 ) == ( angle > 0.0f ) )
			SteerWheel( i, angle );
		else
			SteerWheel( i, m_wheelCount >= 4 ? CalcAckermanAngle( m_steeringAngle, dxFrontWheels, dzAxles ) : m_steeringAngle );
	}
	if ( m_wheelCount > 4 )
	{
		SteerWheel( 4, angle * 0.5f );
		SteerWheel( 5, angle * 0.5f );
	}
}

void CCarSystemRealWheels::FixWheel( int index, bool stop )
{
	Wheel_t &wheel = m_wheels[index];
	if ( !wheel.pObject || !b3Joint_IsValid( wheel.joint ) )
		return;
	if ( !stop )
	{
		wheel.fixed = false;
		b3WheelJoint_EnableSpinMotor( wheel.joint, false );
		return;
	}
	if ( wheel.fixed )
		return;
	// IVP's "magic trick": blend some of the body's spin into the wheel
	// before blocking it, softening the sudden stop.
	b3BodyId wheelBody = wheel.pObject->GetBody();
	Vector wheelOmega = FromB3( b3Body_GetAngularVelocity( wheelBody ) ) + FromB3( b3Body_GetAngularVelocity( m_pBody->GetBody() ) ) * 0.25f;
	b3Body_SetAngularVelocity( wheelBody, ToB3( wheelOmega ) );
	// Block the wheel's spin relative to the body.
	b3WheelJoint_SetSpinMotorSpeed( wheel.joint, 0.0f );
	b3WheelJoint_SetMaxSpinTorque( wheel.joint, FLT_MAX );
	b3WheelJoint_EnableSpinMotor( wheel.joint, true );
	wheel.fixed = true;
}

double CCarSystemRealWheels::GetBodySpeed()
{
	CIvpCore core( m_pBody );
	return DotProduct( core.Axis( 2 ), core.Speed() );
}

void CCarSystemRealWheels::SetPowerslide( float frontAccel, float rearAccel )
{
	float mass = m_pBody->GetMass();
	m_powerslideBack = mass * frontAccel;
	m_powerslideFront = mass * rearAccel;
}

void CCarSystemRealWheels::ActivateBooster( float thrust, float duration, float delay )
{
	if ( m_boosterForward != 0.0f || m_boosterUntilReady > 0.0f )
		return;
	Vector gravity;
	m_pBody->GetEnvironment()->GetGravity( &gravity );
	float g = gravity.Length() * kMetersPerInch;
	float acceleration = thrust * g;
	if ( acceleration != 0.0f )
	{
		m_boosterForward = acceleration * m_pBody->GetMass();
		// IVP's lift: -mass * gravity.y along the body-center-to-far-up line,
		// i.e. mass * g upward.
		m_boosterUp = -m_pBody->GetMass() * g;
	}
	m_boosterToGo = duration;
	m_boosterUntilReady = duration + delay;
}

void CCarSystemRealWheels::UpdateBooster( float dt )
{
	if ( m_boosterUntilReady > 0.0f )
		m_boosterUntilReady -= dt;
	if ( m_boosterToGo > 0.0f )
	{
		m_boosterToGo -= dt;
		if ( m_boosterToGo <= 0.0f )
			m_boosterForward = m_boosterUp = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// CPhysics_Airboat
//-----------------------------------------------------------------------------
#define AIRBOAT_STEERING_RATE_MIN			0.00045f
#define AIRBOAT_STEERING_RATE_MAX			( 5.0f * AIRBOAT_STEERING_RATE_MIN )
#define AIRBOAT_STEERING_INTERVAL			0.5f
#define AIRBOAT_ROT_DRAG					0.00004f
#define AIRBOAT_ROT_DAMPING					0.001f
#define AIRBOAT_THRUST_MAX					11.0f
#define AIRBOAT_THRUST_MAX_REVERSE			7.5f
#define AIRBOAT_WATER_DRAG_LEFT_RIGHT		0.6f
#define AIRBOAT_WATER_DRAG_FORWARD_BACK		0.005f
#define AIRBOAT_WATER_DRAG_UP_DOWN			0.0025f
#define AIRBOAT_GROUND_DRAG_LEFT_RIGHT		2.0
#define AIRBOAT_GROUND_DRAG_FORWARD_BACK	1.0
#define AIRBOAT_GROUND_DRAG_UP_DOWN			0.8
#define AIRBOAT_DRY_FRICTION_SCALE			0.6f
#define AIRBOAT_RAYCAST_DIST				0.35f
#define AIRBOAT_RAYCAST_DIST_WATER_LOW		0.1f
#define AIRBOAT_RAYCAST_DIST_WATER_HIGH		0.35f
#define AIRBOAT_WATER_NOISE_MIN				0.01
#define AIRBOAT_WATER_NOISE_MAX				0.03
#define AIRBOAT_WATER_FREQ_MIN				1.5
#define AIRBOAT_WATER_FREQ_MAX				1.5
#define AIRBOAT_WATER_PHASE_MIN				0.0
#define AIRBOAT_WATER_PHASE_MAX				1.5
#define AIRBOAT_GRAVITY						9.81f
#define AIRBOAT_BUOYANCY_SCALAR				1.6f
#define PONTOON_AREA_2D						2.8f
#define PONTOON_HEIGHT						0.41f
#define AIRBOAT_MAX_WHEELS					4

class CAirboatFrictionData : public IPhysicsCollisionData
{
public:
	CAirboatFrictionData()
	{
		m_vecPoint.Init();
		m_vecNormal.Init();
		m_vecVelocity.Init();
	}
	// IVP's accessors return the point and normal swapped; kept.
	virtual void GetSurfaceNormal( Vector &out ) { out = m_vecPoint; }
	virtual void GetContactPoint( Vector &out ) { out = m_vecNormal; }
	virtual void GetContactSpeed( Vector &out ) { out = m_vecVelocity; }
	Vector m_vecPoint;
	Vector m_vecNormal;
	Vector m_vecVelocity;
};

class CCarSystemAirboat : public CCarSystemBox3D
{
public:
	CCarSystemAirboat( const CarTemplate_t &templ, IPhysicsGameTrace *pGameTrace );
	virtual ~CCarSystemAirboat();

	virtual void Simulate( float dt ) override;
	virtual void DoSteering( float angle, bool analog ) override;
	virtual void ChangeSpringLength( int wheel, float length ) override { m_wheels[wheel].springLength = length; }
	virtual void ChangeWheelTorque( int wheel, float torque ) override { m_wheels[wheel].torque = torque; m_pBody->Wake(); }
	virtual void UpdateThrottle( float throttle ) override;
	virtual void ChangeBodyDownforce( float force ) override { m_downForce = force; }
	virtual void FixWheel( int wheel, bool stop ) override { m_wheels[wheel].fixed = stop; }
	virtual double GetBodySpeed() override;
	virtual double GetWheelAngularVelocity( int wheel ) override { return m_wheels[wheel].angularVelocity; }
	virtual void SetWheelFriction( int wheel, float friction ) override { m_wheels[wheel].frictionOfWheel = friction; }
	virtual void GetDebugData( vehicle_debugcarsystem_t &debug ) override;

private:
	struct Pontoon_t
	{
		Vector raycastStartCS;
		Vector raycastDirCS;
		float raycastLength;
		float springLength;
		float springConstant;
		float springDampRelax;
		float springDampCompress;
		float frictionOfWheel;
		float torque;
		bool fixed;
		Vector axisDirectionCS;
		float angularVelocity;
		float raycastDist;
	};
	struct Impact_t
	{
		float frictionValue;
		Vector raycastDirWS;
		Vector surfaceSpeedWheelWS;
		Vector projectedSurfaceSpeedWheelWS;
		Vector axisDirectionWS;
		Vector projectedAxisDirectionWS;
		float invNormalDotDir;
		bool bImpact;
		bool bImpactWater;
		bool bInWater;
		Vector vecImpactPointWS;
		Vector vecImpactNormalWS;
		float flDepth;
		float flFriction;
		float flDampening;
		int nSurfaceProps;
	};
	struct Ray_s
	{
		Vector start;		// IVP world
		Vector direction;
		float length;
	};

	float ComputeFrontPontoonWaveNoise( int pontoon, float speedRatio );
	void PreRaycastsGameside( Ray_s *pRays, Ray_t *pGameRays, Impact_t *pImpacts );
	float GetWaterDepth( Ray_t *pGameRay );
	void DoRaycastsGameside( Ray_s *pRays, Impact_t *pImpacts );
	void PreRaycasts( Ray_s *pRays, const CIvpCore &core, Impact_t *pImpacts );
	bool PostRaycasts( Ray_s *pRays, const CIvpCore &core, Impact_t *pImpacts );
	void UpdateAirborneState( Impact_t *pImpacts, float dt );
	int CountSurfaceContactPoints( Impact_t *pImpacts );
	void DoSimulationPontoons( const CIvpCore &core, Impact_t *pImpacts, float dt );
	void DoSimulationDrag( const CIvpCore &core, Impact_t *pImpacts, float dt );
	void DoSimulationTurbine( const CIvpCore &core, float dt );
	void DoSimulationSteering( const CIvpCore &core, float dt );
	void DoSimulationKeepUprightPitch( const CIvpCore &core, Impact_t *pImpacts, float dt );
	void DoSimulationKeepUprightRoll( const CIvpCore &core, Impact_t *pImpacts, float dt );
	void PerformFrictionNotification( float eliminatedEnergy, float dt, int surfaceProp, IPhysicsCollisionData *pData );

	CPhysicsObjectBox3D *m_pBody;
	IPhysicsGameTrace *m_pGameTrace;
	Pontoon_t m_wheels[AIRBOAT_MAX_WHEELS];
	int m_wheelCount;
	int m_wheelsPerAxle;
	float m_downForce;
	float m_steeringAngle;
	bool m_steeringReversed;
	bool m_analogSteering;
	float m_prevSteeringAngle;
	float m_steerTime;
	float m_thrust;
	bool m_airborne;
	float m_airTime;
	bool m_weakJump;
	float m_pitchErrorPrev;
	float m_rollErrorPrev;
	float m_time;
	Vector m_localVelocity;		// IVP core axes
	vehicle_debugcarsystem_t m_debug;
};

CCarSystemAirboat::CCarSystemAirboat( const CarTemplate_t &templ, IPhysicsGameTrace *pGameTrace )
{
	m_pBody = templ.pBody;
	m_pGameTrace = pGameTrace;
	m_wheelCount = MIN( templ.wheelCount, AIRBOAT_MAX_WHEELS );
	m_wheelsPerAxle = templ.axleCount ? templ.wheelCount / templ.axleCount : templ.wheelCount;
	m_downForce = 0.0f;
	m_steeringReversed = false;
	m_analogSteering = false;
	m_prevSteeringAngle = 0.0f;
	m_steerTime = 0.0f;
	m_thrust = 0.0f;
	m_airborne = false;
	m_airTime = 0.0f;
	m_weakJump = false;
	m_pitchErrorPrev = m_rollErrorPrev = 0.0f;
	m_time = 0.0f;
	m_localVelocity.Init();
	memset( &m_debug, 0, sizeof( m_debug ) );

	// InitRaycastCarBody: start at rest.
	Vector zero( 0, 0, 0 );
	AngularImpulse zeroAngular( 0, 0, 0 );
	m_pBody->SetVelocity( &zero, &zeroAngular );
	Vector massCenter = m_pBody->GetMassCenterLocalSpace();
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		Pontoon_t &wheel = m_wheels[i];
		memset( &wheel, 0, sizeof( wheel ) );
		wheel.raycastStartCS = HLToIvpPos( templ.tracePositionLocal[i] - massCenter );
		wheel.raycastLength = AIRBOAT_RAYCAST_DIST;
		wheel.raycastDirCS.Init( 0, 1, 0 );		// gravity is +y (down) in IVP
		wheel.springLength = -templ.springPreTension[i];
		wheel.springConstant = templ.springConstant[i];
		wheel.springDampRelax = templ.springDamping[i];
		wheel.springDampCompress = templ.springDampingCompression[i];
		wheel.frictionOfWheel = 1.0f;
		wheel.fixed = true;
		wheel.axisDirectionCS.Init( 1, 0, 0 );
	}
	m_steeringAngle = -1.0f;
	DoSteering( 0.0f, false );

	// InitAirboat: the wheels are not collision objects, the airboat does its
	// own gravity and buoyancy.
	for ( int i = 0; i < templ.wheelCount; i++ )
	{
		if ( !templ.pWheels[i] )
			continue;
		templ.pWheels[i]->EnableCollisions( false );
		// Nothing holds the airboat's wheel objects, so IVP lets them fall
		// forever; they are pinned here instead.
		templ.pWheels[i]->EnableGravity( false );
		templ.pWheels[i]->EnableMotion( false );
	}
	m_pBody->EnableGravity( false );
	m_pBody->SetCallbackFlags( m_pBody->GetCallbackFlags() & ~CALLBACK_DO_FLUID_SIMULATION );
}

CCarSystemAirboat::~CCarSystemAirboat()
{
}

void CCarSystemAirboat::DoSteering( float angle, bool analog )
{
	if ( m_steeringAngle == angle )
		return;
	m_analogSteering = analog;
	m_steeringAngle = angle;
	m_pBody->Wake();
	for ( int i = 0; i < m_wheelsPerAxle && i < m_wheelCount; i++ )
	{
		// Axis x rotated about IVP y by the angle.
		float s = sinf( angle ), c = cosf( angle );
		m_wheels[i].axisDirectionCS.Init( c, 0, -s );
	}
}

void CCarSystemAirboat::UpdateThrottle( float throttle )
{
	if ( fabsf( throttle ) < 0.01f )
		m_thrust = 0.0f;
	else if ( throttle > 0.0f )
		m_thrust = AIRBOAT_THRUST_MAX * throttle;
	else
		m_thrust = AIRBOAT_THRUST_MAX_REVERSE * throttle;
}

double CCarSystemAirboat::GetBodySpeed()
{
	CIvpCore core( m_pBody );
	return DotProduct( core.Axis( 2 ), core.Speed() );
}

void CCarSystemAirboat::GetDebugData( vehicle_debugcarsystem_t &debug )
{
	for ( int i = 0; i < VEHICLE_DEBUGRENDERDATA_MAX_WHEELS; i++ )
	{
		debug.vecWheelRaycasts[i][0] = m_debug.vecWheelRaycasts[i][0];
		debug.vecWheelRaycasts[i][1] = m_debug.vecWheelRaycasts[i][1];
		debug.vecWheelRaycastImpacts[i] = m_debug.vecWheelRaycastImpacts[i];
	}
}

float CCarSystemAirboat::ComputeFrontPontoonWaveNoise( int pontoon, float speedRatio )
{
	float noiseScale = RemapValClamped( 1.0 - speedRatio, 0, 1, AIRBOAT_WATER_NOISE_MIN, AIRBOAT_WATER_NOISE_MAX );
	float phaseShift = 0;
	if ( speedRatio < 0.3 )
		phaseShift = pontoon * AIRBOAT_WATER_PHASE_MAX;
	float frequency = RemapValClamped( speedRatio, 0, 1, AIRBOAT_WATER_FREQ_MIN, AIRBOAT_WATER_FREQ_MAX );
	return noiseScale * sin( frequency * ( m_time + phaseShift ) );
}

void CCarSystemAirboat::PreRaycastsGameside( Ray_s *pRays, Ray_t *pGameRays, Impact_t *pImpacts )
{
	float forwardSpeedRatio = clamp( m_localVelocity.z / 10.0f, 0.f, 1.0f );
	CIvpCore core( m_pBody );
	float speedRatio = clamp( core.Speed().Length() / 15.0f, 0.f, 1.0f );
	if ( !m_thrust )
		forwardSpeedRatio *= 0.5;

	Vector starts[AIRBOAT_MAX_WHEELS], directions[AIRBOAT_MAX_WHEELS];
	Vector zero( 0, 0, 0 );
	int frontInWater = 0;
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		starts[i] = IvpToHLPos( pRays[i].start );
		directions[i] = IvpToHLDir( pRays[i].direction );
		float rayLength = ConvertDistanceToHL( pRays[i].length );
		pImpacts[i].bInWater = false;
		if ( m_pGameTrace->VehiclePointInWater( starts[i] ) )
		{
			directions[i].Negate();
			pImpacts[i].bInWater = true;
		}
		Vector end = starts[i] + directions[i] * rayLength;
		if ( m_pGameTrace->VehiclePointInWater( end ) )
		{
			pRays[i].length = AIRBOAT_RAYCAST_DIST_WATER_LOW;
			if ( i < 2 )
			{
				frontInWater++;
				pRays[i].length += ComputeFrontPontoonWaveNoise( i, speedRatio );
			}
			else
			{
				rayLength = ConvertDistanceToHL( pRays[i].length );
				end = starts[i] + directions[i] * rayLength;
			}
		}
		pGameRays[i].Init( starts[i], end, zero, zero );
	}
	if ( frontInWater == 2 )
	{
		for ( int i = 0; i < 2; i++ )
		{
			pRays[i].length = RemapValClamped( forwardSpeedRatio, 0, 1, AIRBOAT_RAYCAST_DIST_WATER_LOW, AIRBOAT_RAYCAST_DIST_WATER_HIGH );
			pRays[i].length += ComputeFrontPontoonWaveNoise( i, speedRatio );
			Vector end = starts[i] + directions[i] * ConvertDistanceToHL( pRays[i].length );
			pGameRays[i].Init( starts[i], end, zero, zero );
		}
	}
}

float CCarSystemAirboat::GetWaterDepth( Ray_t *pGameRay )
{
	trace_t trace;
	Ray_t waterRay;
	Vector start = pGameRay->m_Start;
	Vector end( start.x, start.y, start.z + 1000.0f );
	Vector zero( 0, 0, 0 );
	waterRay.Init( start, end, zero, zero );
	m_pGameTrace->VehicleTraceRayWithWater( waterRay, m_pBody->GetGameData(), &trace );
	return 1000.0f * trace.fractionleftsolid;
}

void CCarSystemAirboat::DoRaycastsGameside( Ray_s *pRays, Impact_t *pImpacts )
{
	Ray_t gameRays[AIRBOAT_MAX_WHEELS];
	PreRaycastsGameside( pRays, gameRays, pImpacts );
	trace_t trace;
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		if ( pImpacts[i].bInWater )
		{
			m_pGameTrace->VehicleTraceRay( gameRays[i], m_pBody->GetGameData(), &trace );
			pImpacts[i].flDepth = GetWaterDepth( &gameRays[i] );
		}
		else
		{
			m_pGameTrace->VehicleTraceRayWithWater( gameRays[i], m_pBody->GetGameData(), &trace );
		}
		if ( i < VEHICLE_DEBUGRENDERDATA_MAX_WHEELS )
		{
			m_debug.vecWheelRaycasts[i][0] = gameRays[i].m_Start + gameRays[i].m_StartOffset;
			m_debug.vecWheelRaycasts[i][1] = gameRays[i].m_Start + gameRays[i].m_StartOffset + gameRays[i].m_Delta;
			m_debug.vecWheelRaycastImpacts[i] = m_debug.vecWheelRaycasts[i][0] +
				( m_debug.vecWheelRaycasts[i][1] - m_debug.vecWheelRaycasts[i][0] ) * trace.fraction;
		}
		pImpacts[i].bImpactWater = false;
		pImpacts[i].bImpact = false;
		if ( trace.fraction != 1.0f )
		{
			pImpacts[i].bImpact = true;
			pImpacts[i].flDepth = 0.0f;
			if ( trace.contents & MASK_WATER )
				pImpacts[i].bImpactWater = true;
			pImpacts[i].vecImpactPointWS = HLToIvpPos( trace.endpos );
			pImpacts[i].vecImpactNormalWS = HLToIvpDir( trace.plane.normal );
			const surfacedata_t *pSurface = g_SurfaceDatabase.GetSurfaceData( trace.surface.surfaceProps );
			pImpacts[i].nSurfaceProps = trace.surface.surfaceProps;
			pImpacts[i].flDampening = pSurface ? pSurface->physics.dampening : 0.0f;
			pImpacts[i].flFriction = pSurface ? pSurface->physics.friction : 0.8f;
		}
	}
}

void CCarSystemAirboat::PreRaycasts( Ray_s *pRays, const CIvpCore &core, Impact_t *pImpacts )
{
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		pRays[i].start = core.CoreToWorldPos( m_wheels[i].raycastStartCS );
		pImpacts[i].raycastDirWS = core.CoreToWorldDir( m_wheels[i].raycastDirCS );
		pRays[i].direction = pImpacts[i].raycastDirWS;
		pRays[i].length = AIRBOAT_RAYCAST_DIST;
	}
}

// Component of v perpendicular to the normal (IVP set_orthogonal_part).
static Vector OrthogonalPart( const Vector &v, const Vector &normal )
{
	return v - normal * DotProduct( v, normal );
}

bool CCarSystemAirboat::PostRaycasts( Ray_s *pRays, const CIvpCore &core, Impact_t *pImpacts )
{
	bool result = true;
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		Pontoon_t *pWheel = &m_wheels[i];
		Impact_t *pImpact = &pImpacts[i];
		pWheel->raycastLength = pRays[i].length;
		if ( pImpact->bInWater )
			pImpact->raycastDirWS = -pImpact->raycastDirWS;
		if ( pImpact->bImpact )
		{
			pWheel->raycastDist = ( pImpact->vecImpactPointWS - pRays[i].start ).Length();
			pImpact->invNormalDotDir = 1.1f / ( fabsf( DotProduct( pImpact->raycastDirWS, pImpact->vecImpactNormalWS ) ) + 0.1f );
			pImpact->frictionValue = pImpact->flFriction * pWheel->frictionOfWheel;
		}
		else
		{
			pWheel->raycastDist = pWheel->raycastLength;
			pImpact->invNormalDotDir = 1.0f;
			pImpact->vecImpactNormalWS = -pImpact->raycastDirWS;
			pImpact->frictionValue = 1.0f;
		}
		pImpact->vecImpactPointWS = pRays[i].start + pImpact->raycastDirWS * pWheel->raycastDist;
		pImpact->surfaceSpeedWheelWS = core.SurfaceSpeed( pImpact->vecImpactPointWS );
		pImpact->projectedSurfaceSpeedWheelWS = OrthogonalPart( pImpact->surfaceSpeedWheelWS, pImpact->vecImpactNormalWS );
		pImpact->axisDirectionWS = core.CoreToWorldDir( pWheel->axisDirectionCS );
		pImpact->projectedAxisDirectionWS = OrthogonalPart( pImpact->axisDirectionWS, pImpact->vecImpactNormalWS );
		if ( VectorNormalize( pImpact->projectedAxisDirectionWS ) < 1e-6f )
			result = false;
	}
	return result;
}

int CCarSystemAirboat::CountSurfaceContactPoints( Impact_t *pImpacts )
{
	int count = 0;
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		if ( pImpacts[i].bImpact )
			count++;
	}
	return count;
}

void CCarSystemAirboat::UpdateAirborneState( Impact_t *pImpacts, float dt )
{
	if ( !CountSurfaceContactPoints( pImpacts ) )
	{
		if ( !m_airborne )
		{
			m_airborne = true;
			m_airTime = 0;
			CIvpCore core( m_pBody );
			if ( core.Speed().Length() < 11.0f )
				m_weakJump = true;
		}
		else
		{
			m_airTime += dt;
		}
	}
	else
	{
		m_airborne = false;
		m_weakJump = false;
	}
}

void CCarSystemAirboat::DoSimulationPontoons( const CIvpCore &core, Impact_t *pImpacts, float dt )
{
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		Pontoon_t *pWheel = &m_wheels[i];
		Impact_t *pImpact = &pImpacts[i];
		if ( pImpact->bImpact )
		{
			// Ground: a spring pushing along the surface normal.
			double diff = pWheel->raycastDist - pWheel->raycastLength;
			if ( diff >= 0 )
				continue;
			double force = -diff * pWheel->springConstant;
			float invNormalDotDir = clamp( pImpact->invNormalDotDir, 0.0f, 3.0f );
			force *= invNormalDotDir;
			Vector speedDelta = pImpact->projectedSurfaceSpeedWheelWS - pImpact->surfaceSpeedWheelWS;
			double speed = DotProduct( speedDelta, pImpact->raycastDirWS );
			force -= ( speed > 0 ? pWheel->springDampRelax : pWheel->springDampCompress ) * speed;
			if ( force < 0 )
				force = 0.0;
			core.PushWS( pImpact->vecImpactPointWS, pImpact->vecImpactNormalWS * (float)( force * dt ) );
		}
		else if ( pImpact->bInWater )
		{
			float depth = clamp( pImpact->flDepth, 0.f, PONTOON_HEIGHT );
			float submergedVolume = PONTOON_AREA_2D * depth * 0.0254f;
			float force = AIRBOAT_BUOYANCY_SCALAR * 0.25f * core.Mass() * submergedVolume * 1000.0f;
			core.PushWS( pImpact->vecImpactPointWS, Vector( 0, -1, 0 ) * ( force * dt ) );
		}
	}
}

void CCarSystemAirboat::PerformFrictionNotification( float eliminatedEnergy, float dt, int surfaceProp, IPhysicsCollisionData *pData )
{
	if ( ( m_pBody->GetCallbackFlags() & CALLBACK_GLOBAL_FRICTION ) == 0 )
		return;
	IPhysicsCollisionEvent *pHandler = m_pBody->GetEnvironment()->GetCollisionEventHandler();
	if ( !pHandler )
		return;
	eliminatedEnergy *= dt / m_pBody->GetMass();
	if ( eliminatedEnergy > 0.05f )
		pHandler->Friction( m_pBody, eliminatedEnergy, m_pBody->GetMaterialIndex(), surfaceProp, pData );
}

void CCarSystemAirboat::DoSimulationDrag( const CIvpCore &core, Impact_t *pImpacts, float dt )
{
	Vector speedWS = core.Speed();
	float speed = speedWS.Length();
	CAirboatFrictionData frictionData;
	frictionData.m_vecVelocity = IvpToHLDir( speedWS );

	int pointsInWater = 0, pointsOnGround = 0;
	float groundFriction = 0, averageDampening = 0;
	int surfacePropCount[AIRBOAT_MAX_WHEELS], surfaceProp[AIRBOAT_MAX_WHEELS];
	memset( surfacePropCount, 0, sizeof( surfacePropCount ) );
	memset( surfaceProp, 0xFF, sizeof( surfaceProp ) );
	int distinctProps = 0, maxPropIndex = 0;
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		Impact_t *pImpact = &pImpacts[i];
		if ( !pImpact->bImpact )
			continue;
		if ( pImpact->bImpactWater )
		{
			averageDampening += pImpact->flDampening;
			pointsInWater++;
			continue;
		}
		groundFriction += pImpact->flFriction;
		pointsOnGround++;
		int k;
		for ( k = 0; k < distinctProps; k++ )
		{
			if ( surfaceProp[k] == pImpact->nSurfaceProps )
				break;
		}
		if ( k == distinctProps )
			distinctProps++;
		surfaceProp[k] = pImpact->nSurfaceProps;
		if ( ++surfacePropCount[k] > surfacePropCount[maxPropIndex] )
			maxPropIndex = k;
		frictionData.m_vecPoint += IvpToHLPos( pImpact->vecImpactPointWS );
		frictionData.m_vecNormal += IvpToHLDir( pImpact->vecImpactNormalWS );
	}
	int dominantProp = surfaceProp[maxPropIndex];
	if ( pointsOnGround > 0 )
	{
		frictionData.m_vecPoint /= pointsOnGround;
		frictionData.m_vecNormal /= pointsOnGround;
		VectorNormalize( frictionData.m_vecNormal );
	}

	if ( pointsInWater )
	{
		// Directional water drag in local space.
		Vector negative = -m_localVelocity;
		Vector dragLS( AIRBOAT_WATER_DRAG_LEFT_RIGHT * negative.x, AIRBOAT_WATER_DRAG_UP_DOWN * negative.y,
			AIRBOAT_WATER_DRAG_FORWARD_BACK * negative.z );
		dragLS *= speed * core.Mass() * dt;
		core.CenterPushWS( core.CoreToWorldDir( dragLS ) );
	}
	if ( pointsOnGround && speed > 0 )
	{
		groundFriction /= (float)pointsOnGround;
		float frictionDrag = core.Mass() * AIRBOAT_GRAVITY * AIRBOAT_DRY_FRICTION_SCALE * groundFriction / speed;
		float energyBefore = m_pBody->GetEnergy();
		Vector negative = -m_localVelocity;
		Vector dragLS( AIRBOAT_GROUND_DRAG_LEFT_RIGHT * negative.x, AIRBOAT_GROUND_DRAG_UP_DOWN * negative.y,
			AIRBOAT_GROUND_DRAG_FORWARD_BACK * negative.z );
		dragLS *= frictionDrag * dt;
		core.CenterPushWS( core.CoreToWorldDir( dragLS ) );
		PerformFrictionNotification( energyBefore - m_pBody->GetEnergy(), dt, dominantProp, &frictionData );
	}
}

void CCarSystemAirboat::DoSimulationTurbine( const CIvpCore &core, float dt )
{
	float thrust = m_thrust;
	if ( m_weakJump || ( m_airborne && thrust < 0 ) )
		thrust *= 0.5;
	Vector forward = core.Axis( 2 );
	if ( forward.y < -0.5 && thrust > 0 )
		thrust *= 1 + forward.y;	// driving up a slope
	else if ( forward.y > 0.5 && thrust < 0 )
		thrust *= 1 - forward.y;	// reversing up a slope
	core.CenterPushWS( forward * ( thrust * core.Mass() * dt ) );
}

void CCarSystemAirboat::DoSimulationSteering( const CIvpCore &core, float dt )
{
	if ( m_steeringAngle == 0 || m_thrust != 0 )
	{
		float deadZone = m_analogSteering ? 2.0f : 0.0f;
		if ( m_thrust < -deadZone )
			m_steeringReversed = true;
		else if ( m_thrust > deadZone || m_localVelocity.z > 0 )
			m_steeringReversed = false;
	}
	float forceSteering = 0.0f;
	float invDt = 1.0f / dt;
	if ( fabsf( m_steeringAngle ) > 0.01 )
	{
		float steeringSign = m_steeringAngle < 0.0f ? -1.0f : 1.0f;
		if ( m_steeringReversed )
			steeringSign *= -1.0f;
		float prevSign = m_prevSteeringAngle < 0.0f ? -1.0f : 1.0f;
		if ( fabs( m_prevSteeringAngle ) < 0.01 || steeringSign != prevSign )
			m_steerTime = 0;
		float steerScale;
		if ( !m_analogSteering )
			steerScale = RemapValClamped( m_steerTime, 0, AIRBOAT_STEERING_INTERVAL, AIRBOAT_STEERING_RATE_MIN, AIRBOAT_STEERING_RATE_MAX );
		else
			steerScale = RemapValClamped( fabs( m_steeringAngle ), 0, AIRBOAT_STEERING_INTERVAL, AIRBOAT_STEERING_RATE_MIN, AIRBOAT_STEERING_RATE_MAX );
		forceSteering = steerScale * core.Mass() * invDt * -steeringSign;
		m_steerTime += dt;
	}
	m_prevSteeringAngle = m_steeringAngle * ( m_steeringReversed ? -1.0 : 1.0 );

	Vector rotSpeed = core.RotSpeed();
	float sign = rotSpeed.y < 0.0f ? -1.0f : 1.0f;
	float rotationalDrag = AIRBOAT_ROT_DRAG * rotSpeed.y * rotSpeed.y * core.Mass() * invDt * sign;
	float rotationalDamping = AIRBOAT_ROT_DAMPING * fabsf( rotSpeed.y ) * core.Mass() * invDt * sign;
	float forceRotational = forceSteering + rotationalDrag + rotationalDamping;
	core.RotPushCS( Vector( 0, -forceRotational, 0 ) );
}

void CCarSystemAirboat::DoSimulationKeepUprightPitch( const CIvpCore &core, Impact_t *pImpacts, float dt )
{
	if ( m_weakJump )
		return;
	Vector upCS( 0, -cosf( DEG2RAD( 10 ) ), sinf( DEG2RAD( 10 ) ) );
	Vector goalCS = core.WorldToCoreDir( Vector( 0, -1, 0 ) );
	goalCS.x = upCS.x;		// no roll control
	VectorNormalize( goalCS );
	Vector axis = CrossProduct( upCS, goalCS );
	float cosine = DotProduct( upCS, goalCS );
	float sine = VectorNormalize( axis );
	float angle = atan2f( sine, cosine );
	if ( CountSurfaceContactPoints( pImpacts ) > 0 )
	{
		m_pitchErrorPrev = angle;
		return;
	}
	Vector impulse = axis * ( core.Mass() * ( 0.1f * angle + 0.04f / dt * ( angle - m_pitchErrorPrev ) ) );
	m_pitchErrorPrev = angle;
	float length = VectorNormalize( impulse );
	length = MIN( length, DEG2RAD( 1.5f ) * core.Mass() );
	core.RotPushCS( impulse * length );
}

void CCarSystemAirboat::DoSimulationKeepUprightRoll( const CIvpCore &core, Impact_t *pImpacts, float dt )
{
	Vector upCS( 0, -cosf( DEG2RAD( 10 ) ), sinf( DEG2RAD( 10 ) ) );
	Vector goalCS = core.WorldToCoreDir( Vector( 0, -1, 0 ) );
	goalCS.y = upCS.y;		// no pitch control
	VectorNormalize( goalCS );
	Vector axis = CrossProduct( upCS, goalCS );
	float cosine = DotProduct( upCS, goalCS );
	float sine = VectorNormalize( axis );
	float angle = atan2f( sine, cosine );
	if ( CountSurfaceContactPoints( pImpacts ) > 0 || fabsf( angle ) < DEG2RAD( 10 ) )
	{
		m_rollErrorPrev = angle;
		return;
	}
	Vector impulse = axis * ( core.Mass() * ( 0.2f * angle + 0.3f / dt * ( angle - m_rollErrorPrev ) ) );
	m_rollErrorPrev = angle;
	float length = VectorNormalize( impulse );
	length = MIN( length, DEG2RAD( 2.0f ) * core.Mass() );
	core.RotPushCS( impulse * length );
}

void CCarSystemAirboat::Simulate( float dt )
{
	if ( dt <= 0.0f || !m_pBody->IsMoveable() || m_pBody->IsAsleep() )
		return;
	m_time += dt;
	CIvpCore core( m_pBody );
	m_localVelocity = core.WorldToCoreDir( core.Speed() );

	// The airboat's own gravity (IVP_Standard_Gravity_Controller, 9.81 m/s^2
	// along IVP +y) replaces the environment's.
	core.CenterPushWS( Vector( 0, AIRBOAT_GRAVITY * core.Mass() * dt, 0 ) );

	Ray_s rays[AIRBOAT_MAX_WHEELS];
	Impact_t impacts[AIRBOAT_MAX_WHEELS];
	memset( impacts, 0, sizeof( impacts ) );
	PreRaycasts( rays, core, impacts );
	DoRaycastsGameside( rays, impacts );
	if ( !PostRaycasts( rays, core, impacts ) )
		return;
	UpdateAirborneState( impacts, dt );
	DoSimulationPontoons( core, impacts, dt );
	DoSimulationDrag( core, impacts, dt );
	DoSimulationTurbine( core, dt );
	DoSimulationSteering( core, dt );
	DoSimulationKeepUprightPitch( core, impacts, dt );
	DoSimulationKeepUprightRoll( core, impacts, dt );
}

//-----------------------------------------------------------------------------
// Controller (vphysics/physics_vehicle.cpp CVehicleController)
//-----------------------------------------------------------------------------
CVehicleControllerBox3D::CVehicleControllerBox3D()
	: m_pEnv( NULL ), m_pGameTrace( NULL ), m_vehicleType( VEHICLE_TYPE_CAR_WHEELS )
{
	memset( &m_vehicleData, 0, sizeof( m_vehicleData ) );
	ResetState();
}

CVehicleControllerBox3D::CVehicleControllerBox3D( CPhysicsEnvironmentBox3D *pEnv, const vehicleparams_t &params,
	unsigned int vehicleType, IPhysicsGameTrace *pGameTrace )
	: m_pEnv( pEnv ), m_pGameTrace( pGameTrace ), m_vehicleType( vehicleType )
{
	m_vehicleData = params;
	VehicleDataReload();
	ResetState();
}

CVehicleControllerBox3D::~CVehicleControllerBox3D()
{
	ShutdownCarSystem();
}

void CVehicleControllerBox3D::ResetState()
{
	m_pCarSystem = NULL;
	for ( int i = 0; i < VEHICLE_MAX_WHEEL_COUNT; i++ )
	{
		m_pWheels[i] = NULL;
		m_wheelPositionLocal[i].Init();
		m_tracePositionLocal[i].Init();
	}
	m_pBody = NULL;
	m_torqueScale = 1;
	m_wheelCount = 0;
	m_wheelRadius = 0;
	memset( &m_currentState, 0, sizeof( m_currentState ) );
	m_bodyMass = 0;
	m_totalWheelMass = 0;
	m_gravityLength = 0;
	m_vehicleFlags = 0;
	m_traceData = m_vehicleType == VEHICLE_TYPE_AIRBOAT_RAYCAST;
	m_tireType = VEHICLE_TIRE_NORMAL;
	m_occupied = false;
	m_engineDisabled = false;
}

void CVehicleControllerBox3D::VehicleDataReload()
{
	m_torqueScale = 1;
	float totalTorqueDistribution = 0.0f;
	for ( int i = 0; i < m_vehicleData.axleCount; i++ )
		totalTorqueDistribution += m_vehicleData.axles[i].torqueFactor;
	if ( totalTorqueDistribution > 0 )
		m_torqueScale /= totalTorqueDistribution;
	// Script speeds are in miles/hour.
	m_vehicleData.engine.maxSpeed = MPH_TO_GAMEVEL( m_vehicleData.engine.maxSpeed );
	m_vehicleData.engine.maxRevSpeed = MPH_TO_GAMEVEL( m_vehicleData.engine.maxRevSpeed );
	m_vehicleData.engine.boostMaxSpeed = MPH_TO_GAMEVEL( m_vehicleData.engine.boostMaxSpeed );
}

void CVehicleControllerBox3D::InitCarSystem( CPhysicsObjectBox3D *pBody )
{
	if ( m_pCarSystem )
		ShutdownCarSystem();
	m_pBody = pBody;
	m_bodyMass = pBody->GetMass();
	Vector gravity;
	m_pEnv->GetGravity( &gravity );
	m_gravityLength = gravity.Length() * kMetersPerInch;
	m_wheelCount = MIN( m_vehicleData.axleCount * m_vehicleData.wheelsPerAxle, VEHICLE_MAX_WHEEL_COUNT );
	CreateCarSystem();
	if ( m_vehicleType == VEHICLE_TYPE_AIRBOAT_RAYCAST )
	{
		float damp = 1.0f, rotDamp = 1.0f;
		m_pBody->SetDamping( &damp, &rotDamp );
	}
}

CPhysicsObjectBox3D *CVehicleControllerBox3D::CreateWheel( int wheelIndex, const vehicle_axleparams_t &axle )
{
	if ( wheelIndex >= VEHICLE_MAX_WHEEL_COUNT )
		return NULL;
	Vector local = axle.offset + ( ( wheelIndex & 1 ) ? axle.wheelOffset : -axle.wheelOffset );
	m_wheelPositionLocal[wheelIndex] = local;
	if ( m_traceData )
		m_tracePositionLocal[wheelIndex] = axle.raytraceCenterOffset + ( ( wheelIndex & 1 ) ? axle.raytraceOffset : -axle.raytraceOffset );
	// Restored wheels already exist.
	if ( m_pWheels[wheelIndex] )
		return m_pWheels[wheelIndex];

	Vector bodyPosition;
	QAngle bodyAngles;
	m_pBody->GetPosition( &bodyPosition, &bodyAngles );
	Vector position;
	m_pBody->LocalToWorld( &position, local );

	objectparams_t params;
	memset( &params, 0, sizeof( params ) );
	params.damping = axle.wheels.damping;
	params.dragCoefficient = 0;
	params.enableCollisions = false;
	params.inertia = axle.wheels.inertia;
	params.mass = axle.wheels.mass;
	params.pGameData = m_pBody->GetGameData();
	params.pName = "VehicleWheel";
	params.rotdamping = axle.wheels.rotdamping;
	params.rotInertiaLimit = 0;
	float radius = axle.wheels.radius;
	params.volume = ( 4 / 3 ) * M_PI * radius * radius * radius;	// IVP's integer 4/3, kept
	CPhysicsObjectBox3D *pWheel = ToBox3D( m_pEnv->CreateSphereObject( radius, axle.wheels.materialIndex, position, bodyAngles, &params, false ) );
	if ( !pWheel )
		return NULL;
	pWheel->Wake();
	pWheel->SetCallbackFlags( CALLBACK_IS_VEHICLE_WHEEL );
	pWheel->SetGameFlags( m_pBody->GetGameFlags() );
	m_pWheels[wheelIndex] = pWheel;
	return pWheel;
}

void CVehicleControllerBox3D::InitCarSystemWheels()
{
	m_wheelRadius = 0;
	m_totalWheelMass = 0;
	int wheelIndex = 0;
	for ( int i = 0; i < m_vehicleData.axleCount; i++ )
	{
		for ( int w = 0; w < m_vehicleData.wheelsPerAxle && wheelIndex < VEHICLE_MAX_WHEEL_COUNT; w++, wheelIndex++ )
		{
			if ( CreateWheel( wheelIndex, m_vehicleData.axles[i] ) )
				m_totalWheelMass += m_vehicleData.axles[i].wheels.mass;
		}
		float radius = ConvertDistanceToIVP( m_vehicleData.axles[i].wheels.radius );
		if ( radius > m_wheelRadius )
			m_wheelRadius = radius;
	}
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		if ( m_pWheels[i] )
			m_pWheels[i]->EnableCollisions( true );
	}
}

void CVehicleControllerBox3D::CreateCarSystem()
{
	InitCarSystemWheels();

	CarTemplate_t templ;
	memset( &templ, 0, sizeof( templ ) );
	templ.wheelCount = m_wheelCount;
	templ.axleCount = m_vehicleData.axleCount;
	templ.pBody = m_pBody;
	templ.bodyCounterTorqueFactor = m_vehicleData.body.counterTorqueFactor;
	templ.bodyDownForceVerticalOffset = ConvertDistanceToIVP( m_vehicleData.body.tiltForceHeight );
	templ.extraGravityForce = m_vehicleData.body.addGravity * m_gravityLength * m_bodyMass;
	int wheelIndex = 0;
	for ( int i = 0; i < m_vehicleData.axleCount && i < VEHICLE_MAX_AXLE_COUNT; i++ )
	{
		const vehicle_axleparams_t &axle = m_vehicleData.axles[i];
		for ( int w = 0; w < m_vehicleData.wheelsPerAxle && wheelIndex < VEHICLE_MAX_WHEEL_COUNT; w++, wheelIndex++ )
		{
			templ.pWheels[wheelIndex] = m_pWheels[wheelIndex];
			templ.wheelPositionLocal[wheelIndex] = m_wheelPositionLocal[wheelIndex];
			templ.tracePositionLocal[wheelIndex] = m_tracePositionLocal[wheelIndex];
			templ.wheelRadius[wheelIndex] = ConvertDistanceToIVP( axle.wheels.radius );
			templ.frictionOfWheel[wheelIndex] = axle.wheels.frictionScale;
			templ.springConstant[wheelIndex] = axle.suspension.springConstant * m_bodyMass;
			templ.springDamping[wheelIndex] = axle.suspension.springDamping * m_bodyMass;
			templ.springDampingCompression[wheelIndex] = axle.suspension.springDampingCompression * m_bodyMass;
			templ.maxBodyForce[wheelIndex] = axle.suspension.maxBodyForce * m_bodyMass;
			templ.springPreTension[wheelIndex] = -ConvertDistanceToIVP( axle.wheels.springAdditionalLength );
		}
		templ.stabilizerConstant[i] = axle.suspension.stabilizerConstant * m_bodyMass;
		float radius = ConvertDistanceToIVP( axle.wheels.radius );
		float totalMaxSpeed = MAX( m_vehicleData.engine.boostMaxSpeed, m_vehicleData.engine.maxSpeed );
		// IVP divides a game-unit speed by a radius in meters; kept.
		templ.wheelMaxRotationSpeed[i] = radius > 0 ? totalMaxSpeed / radius : 0;
	}

	switch ( m_vehicleType )
	{
	case VEHICLE_TYPE_CAR_WHEELS:
	case VEHICLE_TYPE_CAR_RAYCAST:
		m_pCarSystem = new CCarSystemRealWheels( templ );
		break;
	case VEHICLE_TYPE_AIRBOAT_RAYCAST:
		m_pCarSystem = new CCarSystemAirboat( templ, m_pGameTrace );
		break;
	default:
		// IVP builds no car system for other types either.
		m_pCarSystem = NULL;
		break;
	}
}

void CVehicleControllerBox3D::ShutdownCarSystem()
{
	delete m_pCarSystem;
	m_pCarSystem = NULL;
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		if ( m_pWheels[i] && m_pEnv )
			m_pEnv->DestroyObject( m_pWheels[i] );
		m_pWheels[i] = NULL;
	}
}

void CVehicleControllerBox3D::ObjectDestroyed( IPhysicsObject *pObject )
{
	if ( pObject == m_pBody )
	{
		// IVP's object listener: the car system goes with its body.
		ShutdownCarSystem();
		m_pBody = NULL;
		return;
	}
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		if ( m_pWheels[i] == pObject )
		{
			if ( m_pCarSystem )
				m_pCarSystem->ObjectDestroyed( pObject );
			m_pWheels[i] = NULL;
		}
	}
}

void CVehicleControllerBox3D::Simulate( float dt )
{
	if ( m_pCarSystem && m_pBody )
		m_pCarSystem->Simulate( dt );
}

IPhysicsObject *CVehicleControllerBox3D::GetWheel( int index )
{
	if ( index < 0 || index >= m_wheelCount )
		return NULL;
	return m_pWheels[index];
}

void CVehicleControllerBox3D::SetWheelFriction( int wheelIndex, float friction )
{
	if ( m_pCarSystem && m_vehicleType == VEHICLE_TYPE_AIRBOAT_RAYCAST )
		m_pCarSystem->SetWheelFriction( wheelIndex, friction );
}

bool CVehicleControllerBox3D::GetWheelContactPoint( int index, Vector *pContactPoint, int *pSurfaceProps )
{
	bool set = false;
	if ( index < m_wheelCount && m_pWheels[index] )
	{
		IPhysicsFrictionSnapshot *pSnapshot = m_pWheels[index]->CreateFrictionSnapshot();
		float forceMax = -1.0f;
		if ( pContactPoint )
			m_pWheels[index]->GetPosition( pContactPoint, NULL );
		while ( pSnapshot->IsValid() )
		{
			float force = pSnapshot->GetNormalForce();
			if ( force > forceMax )
			{
				forceMax = force;
				if ( pContactPoint )
					pSnapshot->GetContactPoint( *pContactPoint );
				if ( pSurfaceProps )
					*pSurfaceProps = pSnapshot->GetMaterial( 1 );
				set = true;
			}
			pSnapshot->NextFrictionData();
		}
		m_pWheels[index]->DestroyFrictionSnapshot( pSnapshot );
	}
	else
	{
		if ( pContactPoint )
			pContactPoint->Init();
		if ( pSurfaceProps )
			*pSurfaceProps = 0;
	}
	return set;
}

void CVehicleControllerBox3D::SetSpringLength( int wheelIndex, float length )
{
	if ( m_pCarSystem )
		m_pCarSystem->ChangeSpringLength( wheelIndex, length );
}

float CVehicleControllerBox3D::UpdateBooster( float dt )
{
	if ( !m_pCarSystem )
		return 0.0f;
	m_pCarSystem->UpdateBooster( dt );
	m_currentState.boostDelay = m_pCarSystem->GetBoosterDelay();
	return m_currentState.boostDelay;
}

bool CVehicleControllerBox3D::IsBoosting( void )
{
	return m_pCarSystem && m_pCarSystem->GetBoosterTimeToGo() > 0.0f;
}

void CVehicleControllerBox3D::Update( float dt, vehicle_controlparams_t &controlsIn )
{
	if ( !m_pCarSystem || !m_pBody )
		return;
	vehicle_controlparams_t controls = controlsIn;
	m_currentState.speed = ConvertDistanceToHL( (float)m_pCarSystem->GetBodySpeed() );
	float speed = GAMEVEL_TO_MPH( m_currentState.speed );
	float absSpeed = fabsf( speed );

	float throttle = controls.throttle;
	bool handbrake = controls.handbrake;
	float brake = controls.brake;
	bool powerslide = handbrake && ( absSpeed > 18.0f );
	if ( handbrake )
		throttle = 0.0f;
	if ( IsBoosting() )
	{
		controls.boost = true;
		throttle = throttle < 0.0f ? -1.0f : 1.0f;
	}
	if ( throttle == 0.0f && brake == 0.0f && !handbrake )
		brake = 0.1f;

	UpdateSteering( controls, dt, absSpeed );
	UpdatePowerslide( controls, powerslide, speed );
	UpdateEngine( controls, dt, throttle, brake, handbrake, powerslide );
	UpdateHandbrake( controls, throttle, handbrake, powerslide );
	UpdateSkidding( handbrake );
	UpdateExtraForces();
	UpdateWheelPositions();
}

void CVehicleControllerBox3D::UpdateSteering( const vehicle_controlparams_t &controls, float flDeltaTime, float flSpeed )
{
	float angle = CalcSteering( flDeltaTime, flSpeed, controls.steering, controls.bAnalogSteering );
	m_pCarSystem->DoSteering( DEG2RAD( angle ), controls.bAnalogSteering );
	m_currentState.steeringAngle = angle;
}

void CVehicleControllerBox3D::UpdatePowerslide( const vehicle_controlparams_t &controls, bool bPowerslide, float flSpeed )
{
	if ( !m_vehicleData.steering.isSkidAllowed || !m_occupied )
		return;
	bool left = bPowerslide && controls.handbrakeLeft;
	bool right = bPowerslide && controls.handbrakeRight;
	unsigned int newTireType = VEHICLE_TIRE_NORMAL;
	if ( left || right )
		newTireType = VEHICLE_TIRE_POWERSLIDE;
	else if ( bPowerslide )
		newTireType = VEHICLE_TIRE_BRAKING;
	if ( newTireType != m_tireType )
	{
		int wheel = 0;
		for ( int axle = 0; axle < m_vehicleData.axleCount; axle++ )
		{
			int material = m_vehicleData.axles[axle].wheels.materialIndex;
			if ( newTireType == VEHICLE_TIRE_POWERSLIDE && m_vehicleData.axles[axle].wheels.skidMaterialIndex != -1 )
				material = m_vehicleData.axles[axle].wheels.skidMaterialIndex;
			else if ( newTireType == VEHICLE_TIRE_BRAKING && m_vehicleData.axles[axle].wheels.brakeMaterialIndex != -1 )
				material = m_vehicleData.axles[axle].wheels.brakeMaterialIndex;
			for ( int w = 0; w < m_vehicleData.wheelsPerAxle && wheel < m_wheelCount; w++, wheel++ )
			{
				if ( m_pWheels[wheel] )
					m_pWheels[wheel]->SetMaterialIndex( material );
			}
			m_tireType = newTireType;
		}
	}
	float frontAccel = 0.0f, rearAccel = 0.0f;
	if ( flSpeed > 0 && left != right )
	{
		// Positive acceleration is to the left.
		float amount = RemapValClamped( flSpeed, m_vehicleData.steering.speedSlow, m_vehicleData.steering.speedFast, 0, 1 );
		float accel = ConvertDistanceToIVP( m_vehicleData.steering.powerSlideAccel );
		frontAccel = ( left ? accel : -accel ) * amount;
		rearAccel = -frontAccel;
	}
	m_pCarSystem->SetPowerslide( frontAccel, rearAccel );
}

void CVehicleControllerBox3D::UpdateEngine( const vehicle_controlparams_t &controls, float flDeltaTime, float flThrottle, float flBrake,
	bool bHandbrake, bool bPowerslide )
{
	bool torqueBoost = UpdateEngineTurboStart( controls, flDeltaTime );
	CalcEngine( flThrottle, flBrake, bHandbrake, controls.steering, torqueBoost );
	UpdateEngineTurboFinish();
}

bool CVehicleControllerBox3D::UpdateEngineTurboStart( const vehicle_controlparams_t &controls, float flDeltaTime )
{
	bool torqueBoost = false;
	if ( controls.boost > 0 )
	{
		if ( m_vehicleData.engine.torqueBoost )
		{
			torqueBoost = true;
			m_pCarSystem->ActivateBooster( 0.0f, m_vehicleData.engine.boostDuration, m_vehicleData.engine.boostDelay );
		}
		else
		{
			m_pCarSystem->ActivateBooster( m_vehicleData.engine.boostForce * controls.boost, m_vehicleData.engine.boostDuration,
				m_vehicleData.engine.boostDelay );
		}
	}
	m_pCarSystem->UpdateBooster( flDeltaTime );
	m_currentState.boostDelay = m_pCarSystem->GetBoosterDelay();
	m_currentState.isTorqueBoosting = torqueBoost;
	return torqueBoost;
}

void CVehicleControllerBox3D::UpdateEngineTurboFinish( void )
{
	float total = m_vehicleData.engine.boostDuration + m_vehicleData.engine.boostDelay;
	if ( total > 0 )
	{
		if ( m_currentState.boostDelay > 0 )
			m_currentState.boostTimeLeft = 100 - 100 * ( m_currentState.boostDelay / total );
		else
			m_currentState.boostTimeLeft = 100;
	}
}

void CVehicleControllerBox3D::UpdateHandbrake( const vehicle_controlparams_t &controls, float flThrottle, bool bHandbrake, bool bPowerslide )
{
	m_currentState.speed = ConvertDistanceToHL( (float)m_pCarSystem->GetBodySpeed() );
	if ( !bPowerslide )
	{
		// Lets low throttle overcome gravity.
		if ( ( flThrottle < 0.0f && m_currentState.speed > THROTTLE_OPPOSING_FORCE_EPSILON ) ||
			( flThrottle > 0.0f && m_currentState.speed < -THROTTLE_OPPOSING_FORCE_EPSILON ) )
			bHandbrake = true;
	}
	if ( bHandbrake )
	{
		// Only with wheels on something.
		bHandbrake = false;
		for ( int i = 0; i < m_wheelCount; i++ )
		{
			if ( m_pWheels[i] && m_pWheels[i]->GetContactPoint( NULL, NULL ) )
			{
				bHandbrake = true;
				break;
			}
		}
	}
	bool current = ( m_vehicleFlags & FVEHICLE_HANDBRAKE_ON ) != 0;
	if ( bHandbrake != current )
	{
		if ( bHandbrake )
			m_vehicleFlags |= FVEHICLE_HANDBRAKE_ON;
		else
			m_vehicleFlags &= ~FVEHICLE_HANDBRAKE_ON;
		for ( int i = 0; i < m_wheelCount; i++ )
			m_pCarSystem->FixWheel( i, bHandbrake );
	}
}

void CVehicleControllerBox3D::UpdateSkidding( bool bHandbrake )
{
	m_currentState.skidSpeed = 0.0f;
	m_currentState.skidMaterial = 0;
	m_currentState.wheelsInContact = m_wheelCount;
	m_currentState.wheelsNotInContact = 0;
	if ( !m_vehicleData.steering.isSkidAllowed )
		return;
	float absSpeed = fabsf( m_currentState.speed );
	m_currentState.wheelsInContact = 0;
	m_currentState.wheelsNotInContact = 0;
	for ( int i = 0; i < m_wheelCount; i++ )
	{
		Vector contact, velocity;
		int surfaceProps;
		if ( m_pWheels[i] && GetWheelContactPoint( i, &contact, &surfaceProps ) )
		{
			m_pWheels[i]->GetVelocityAtPoint( contact, &velocity );
			float speed = velocity.Length();
			if ( speed > m_currentState.skidSpeed || m_currentState.skidSpeed <= 0.0f )
			{
				m_currentState.skidSpeed = speed;
				m_currentState.skidMaterial = surfaceProps;
			}
			m_currentState.wheelsInContact++;
		}
		else
		{
			m_currentState.wheelsNotInContact++;
		}
	}
	if ( bHandbrake && absSpeed > 30 )
		m_currentState.skidSpeed = absSpeed;
}

void CVehicleControllerBox3D::UpdateExtraForces( void )
{
	// Down force while the body lies on its side: the body's vertical axis
	// is nearly horizontal.
	Vector bodyUp = FromB3( b3Body_GetWorldVector( m_pBody->GetBody(), b3Vec3{ 0, 0, 1 } ) );
	if ( fabsf( bodyUp.z ) < 0.05f )
		m_pCarSystem->ChangeBodyDownforce( m_vehicleData.body.tiltForce * m_gravityLength * m_bodyMass );
	else
		m_pCarSystem->ChangeBodyDownforce( 0.0f );

	if ( m_vehicleType == VEHICLE_TYPE_CAR_WHEELS )
		m_pCarSystem->UpdateBodyCountertorque();

	AngularImpulse angular;
	m_pBody->GetVelocity( NULL, &angular );
	if ( m_vehicleData.body.maxAngularVelocity > 0 && angular.Length() > m_vehicleData.body.maxAngularVelocity )
	{
		VectorNormalize( angular );
		angular *= m_vehicleData.body.maxAngularVelocity;
		m_pBody->SetVelocityInstantaneous( NULL, &angular );
	}
}

void CVehicleControllerBox3D::UpdateWheelPositions( void )
{
	if ( m_vehicleType == VEHICLE_TYPE_CAR_RAYCAST )
		m_pCarSystem->UpdateWheelPositions();
}

float CVehicleControllerBox3D::CalcSteering( float dt, float speed, float steering, bool bAnalog )
{
	float degrees = RemapValClamped( speed, m_vehicleData.steering.speedSlow, m_vehicleData.steering.speedFast,
		m_vehicleData.steering.degreesSlow, m_vehicleData.steering.degreesFast );
	float speedGame = MPH_TO_GAMEVEL( speed );
	if ( speedGame > m_vehicleData.engine.maxSpeed )
	{
		degrees = RemapValClamped( speedGame, m_vehicleData.engine.maxSpeed, m_vehicleData.engine.boostMaxSpeed,
			m_vehicleData.steering.degreesFast, m_vehicleData.steering.degreesBoost );
	}
	if ( m_vehicleData.steering.steeringExponent != 0 )
	{
		float sign = steering < 0 ? -1 : 1;
		float absSteering = fabsf( steering );
		if ( bAnalog )
		{
			float output = powf( absSteering, 2.0f ) * sign * m_vehicleData.steering.degreesSlow;
			return clamp( output, -degrees, degrees );
		}
		return powf( absSteering, m_vehicleData.steering.steeringExponent ) * sign * degrees;
	}
	return steering * degrees;
}

void CVehicleControllerBox3D::CalcEngineTransmission( float flThrottle )
{
	if ( !m_vehicleData.engine.isAutoTransmission || m_wheelCount <= 0 )
		return;
	float averageRotSpeed = 0.0f;
	for ( int i = 0; i < m_wheelCount; i++ )
		averageRotSpeed += fabsf( (float)m_pCarSystem->GetWheelAngularVelocity( i ) );
	averageRotSpeed *= 0.5f / kPi / m_wheelCount;
	int &gear = m_currentState.gear;
	gear = clamp( gear, 0, MAX( m_vehicleData.engine.gearCount - 1, 0 ) );
	float rpm = averageRotSpeed * m_vehicleData.engine.axleRatio * m_vehicleData.engine.gearRatio[gear] * 60;
	if ( flThrottle > 0.0f )
	{
		while ( rpm > m_vehicleData.engine.shiftUpRPM && gear < m_vehicleData.engine.gearCount - 1 )
		{
			gear++;
			rpm = averageRotSpeed * m_vehicleData.engine.axleRatio * m_vehicleData.engine.gearRatio[gear] * 60;
		}
	}
	while ( rpm < m_vehicleData.engine.shiftDownRPM && gear > 0 )
	{
		gear--;
		rpm = averageRotSpeed * m_vehicleData.engine.axleRatio * m_vehicleData.engine.gearRatio[gear] * 60;
	}
	m_currentState.engineRPM = rpm;
}

void CVehicleControllerBox3D::CalcEngine( float throttle, float brake_val, bool handbrake, float steeringVal, bool torqueBoost )
{
	CalcEngineTransmission( throttle );
	float absSpeed = fabsf( m_currentState.speed );

	// Speed governor (the PC path).
	float maxSpeed = torqueBoost ? m_vehicleData.engine.boostMaxSpeed : m_vehicleData.engine.maxSpeed;
	maxSpeed = MAX( 1.f, maxSpeed );
	if ( throttle > 0 && absSpeed > maxSpeed )
	{
		float frac = absSpeed / maxSpeed;
		if ( frac > m_vehicleData.engine.autobrakeSpeedGain )
		{
			throttle = 0;
			brake_val = ( frac - 1.0f ) * m_vehicleData.engine.autobrakeSpeedFactor;
			if ( m_currentState.wheelsInContact == 0 )
				brake_val = 0;
		}
		throttle *= 0.1f;
	}
	if ( throttle < 0 && !torqueBoost && absSpeed > m_vehicleData.engine.maxRevSpeed )
		throttle *= 0.1f;

	if ( throttle != 0.0 )
	{
		m_vehicleFlags &= ~FVEHICLE_THROTTLE_STOPPED;
		const float wattPerHp = 745.0f;
		const float secondsPerMinute = 60.0f;
		int gear = clamp( m_currentState.gear, 0, VEHICLE_MAX_GEAR_COUNT - 1 );
		float wheelForceByThrottle = 0.0f;
		if ( m_vehicleData.engine.maxRPM > 0 && m_wheelRadius > 0 )
		{
			wheelForceByThrottle = throttle * m_vehicleData.engine.horsepower * ( wattPerHp * secondsPerMinute ) *
				m_vehicleData.engine.gearRatio[gear] * m_vehicleData.engine.axleRatio /
				( m_vehicleData.engine.maxRPM * m_wheelRadius * ( 2 * kPi ) );
		}
		if ( m_currentState.engineRPM >= m_vehicleData.engine.maxRPM )
			wheelForceByThrottle = 0;
		int wheelIndex = 0;
		for ( int i = 0; i < m_vehicleData.axleCount; i++ )
		{
			float axleFactor = m_vehicleData.axles[i].torqueFactor * m_torqueScale;
			float boostFactor = 0.5f;
			if ( torqueBoost && IsBoosting() )
			{
				// Full boost only when driving straight and fast.
				float speedFactor = RemapValClamped( absSpeed, 0, m_vehicleData.engine.maxSpeed, 0.1f, 1.0f );
				float turnFactor = 1.0f - ( fabsf( steeringVal ) * 0.95f );
				float dampedBoost = m_vehicleData.engine.boostForce * speedFactor * turnFactor;
				if ( dampedBoost > boostFactor )
					boostFactor = dampedBoost;
			}
			float axleTorque = boostFactor * wheelForceByThrottle * axleFactor * ConvertDistanceToIVP( m_vehicleData.axles[i].wheels.radius );
			for ( int w = 0; w < m_vehicleData.wheelsPerAxle && wheelIndex < m_wheelCount; w++, wheelIndex++ )
				m_pCarSystem->ChangeWheelTorque( wheelIndex, axleTorque );
		}
	}
	else if ( brake_val != 0 )
	{
		m_vehicleFlags &= ~FVEHICLE_THROTTLE_STOPPED;
		float wheelForceByBrake = brake_val * m_gravityLength * ( m_bodyMass + m_totalWheelMass );
		float sign = m_currentState.speed >= 0.0f ? -1.0f : 1.0f;
		int wheelIndex = 0;
		for ( int i = 0; i < m_vehicleData.axleCount; i++ )
		{
			float torque = 0.5f * sign * wheelForceByBrake * m_vehicleData.axles[i].brakeFactor *
				ConvertDistanceToIVP( m_vehicleData.axles[i].wheels.radius );
			for ( int w = 0; w < m_vehicleData.wheelsPerAxle && wheelIndex < m_wheelCount; w++, wheelIndex++ )
				m_pCarSystem->ChangeWheelTorque( wheelIndex, torque );
		}
	}
	else if ( !( m_vehicleFlags & FVEHICLE_THROTTLE_STOPPED ) )
	{
		m_vehicleFlags |= FVEHICLE_THROTTLE_STOPPED;
		for ( int w = 0; w < m_wheelCount; w++ )
			m_pCarSystem->ChangeWheelTorque( w, 0 );
	}
	// The airboat's turbine reads the throttle.
	m_pCarSystem->UpdateThrottle( throttle );
}

void CVehicleControllerBox3D::GetCarSystemDebugData( vehicle_debugcarsystem_t &debugCarSystem )
{
	memset( &debugCarSystem, 0, sizeof( debugCarSystem ) );
	if ( m_pCarSystem )
		m_pCarSystem->GetDebugData( debugCarSystem );
}

void CVehicleControllerBox3D::OnVehicleEnter( void )
{
	m_occupied = true;
	if ( m_vehicleType == VEHICLE_TYPE_AIRBOAT_RAYCAST && m_pBody )
	{
		float damp = 0.0f, rotDamp = 0.0f;
		m_pBody->SetDamping( &damp, &rotDamp );
	}
}

void CVehicleControllerBox3D::OnVehicleExit( void )
{
	m_occupied = false;
	if ( m_vehicleData.steering.isSkidAllowed && m_pCarSystem )
	{
		int wheel = 0;
		for ( int axle = 0; axle < m_vehicleData.axleCount; axle++ )
		{
			for ( int w = 0; w < m_vehicleData.wheelsPerAxle && wheel < m_wheelCount; w++, wheel++ )
			{
				if ( m_tireType != VEHICLE_TIRE_NORMAL && m_pWheels[wheel] )
					m_pWheels[wheel]->SetMaterialIndex( m_vehicleData.axles[axle].wheels.materialIndex );
				m_pCarSystem->FixWheel( wheel, true );
			}
		}
		m_tireType = VEHICLE_TIRE_NORMAL;
		m_currentState.skidSpeed = 0.0f;
	}
	if ( m_vehicleType == VEHICLE_TYPE_AIRBOAT_RAYCAST && m_pBody )
	{
		float damp = 1.0f, rotDamp = 1.0f;
		m_pBody->SetDamping( &damp, &rotDamp );
	}
	SetEngineDisabled( false );
}

void CVehicleControllerBox3D::WriteState( CVehicleStateBox3D &state )
{
	// The handbrake re-engages on restore (IVP clears the flag so the wheels
	// are fixed again).
	m_vehicleFlags &= ~FVEHICLE_HANDBRAKE_ON;
	memset( &state, 0, sizeof( state ) );
	state.version = 1;
	state.pBody = m_pBody;
	for ( int i = 0; i < VEHICLE_MAX_WHEEL_COUNT; i++ )
		state.pWheels[i] = m_pWheels[i];
	state.wheelCount = m_wheelCount;
	state.vehicleData = m_vehicleData;
	state.currentState = m_currentState;
	state.wheelRadius = m_wheelRadius;
	state.bodyMass = m_bodyMass;
	state.totalWheelMass = m_totalWheelMass;
	state.gravityLength = m_gravityLength;
	state.torqueScale = m_torqueScale;
	state.vehicleFlags = m_vehicleFlags;
	state.tireType = m_tireType;
	state.vehicleType = m_vehicleType;
	state.occupied = m_occupied;
	state.engineDisable = m_engineDisabled;
	if ( m_pBody )
		m_pBody->GetVelocity( &state.bodyVelocity, NULL );
}

CVehicleControllerBox3D *CVehicleControllerBox3D::CreateFromState( CPhysicsEnvironmentBox3D *pEnv, IPhysicsGameTrace *pGameTrace,
	const CVehicleStateBox3D &state )
{
	CVehicleControllerBox3D *pController = new CVehicleControllerBox3D;
	pController->m_pEnv = pEnv;
	pController->m_pGameTrace = pGameTrace;
	pController->m_vehicleType = state.vehicleType;
	pController->ResetState();
	pController->m_vehicleData = state.vehicleData;
	pController->m_currentState = state.currentState;
	pController->m_wheelCount = MIN( state.wheelCount, VEHICLE_MAX_WHEEL_COUNT );
	pController->m_torqueScale = state.torqueScale;
	pController->m_vehicleFlags = state.vehicleFlags;
	pController->m_tireType = state.tireType;
	pController->m_occupied = state.occupied;
	pController->m_engineDisabled = state.engineDisable;
	pController->m_pBody = ToBox3D( state.pBody );
	for ( int i = 0; i < VEHICLE_MAX_WHEEL_COUNT; i++ )
		pController->m_pWheels[i] = state.pWheels[i] ? ToBox3D( state.pWheels[i] ) : NULL;
	if ( pController->m_pBody )
	{
		pController->m_bodyMass = state.bodyMass;
		pController->m_gravityLength = state.gravityLength;
		pController->CreateCarSystem();
		pController->m_wheelRadius = state.wheelRadius;
		pController->m_totalWheelMass = state.totalWheelMass;
		// As IVP: start without spin, keep the linear velocity.
		AngularImpulse zero( 0, 0, 0 );
		pController->m_pBody->SetVelocity( &state.bodyVelocity, &zero );
	}
	return pController;
}
