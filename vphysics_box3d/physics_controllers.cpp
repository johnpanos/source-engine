//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D shadow, player and motion controllers, constraints and
//          friction snapshots (RFC 0004 B5/C3/C4).
//
//=============================================================================//
#include "physics_controllers.h"

#include <math.h>

#include "box3d/box3d.h"
#include "box3d_convert.h"
#include "physics_environment.h"
#include "physics_object.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
// IVP's ComputeController (vphysics/physics_shadow.cpp): accelerate toward the
// scaled error within maxSpeed, and damp the current speed within maxDampSpeed.
void ComputeController( Vector &currentSpeed, const Vector &delta, float maxSpeed, float maxDampSpeed,
	float scaleDelta, float damping, Vector *pOutImpulse )
{
	if ( currentSpeed.LengthSqr() < 1e-6f )
		currentSpeed.Init();

	Vector acceleration( 0, 0, 0 );
	if ( maxSpeed > 0 )
	{
		acceleration = delta * scaleDelta;
		float speed = acceleration.Length();
		if ( speed > maxSpeed )
			acceleration *= maxSpeed / speed;
	}

	Vector dampAccel( 0, 0, 0 );
	if ( maxDampSpeed > 0 )
	{
		dampAccel = currentSpeed * -damping;
		float speed = dampAccel.Length();
		if ( speed > maxDampSpeed )
			dampAccel *= maxDampSpeed / speed;
	}
	currentSpeed += dampAccel + acceleration;
	if ( pOutImpulse )
		*pOutImpulse = acceleration;
}

// World-space rotation (axis * radians) that takes current to target.
Vector RotationError( const b3Quat &current, const b3Quat &target )
{
	b3Quat delta = b3MulQuat( target, b3Conjugate( current ) );
	if ( delta.s < 0.0f )
	{
		delta.s = -delta.s;
		delta.v = b3Neg( delta.v );
	}
	float s = delta.s > 1.0f ? 1.0f : delta.s;
	float angle = 2.0f * acosf( s );
	Vector axis = FromB3( delta.v );
	if ( VectorNormalize( axis ) < 1e-6f )
		return vec3_origin;
	return axis * angle;
}

void TeleportUp( CPhysicsObjectBox3D *pObject, float height )
{
	Vector position;
	QAngle angles;
	pObject->GetPosition( &position, &angles );
	position.z += height;
	pObject->TeleportTo( position, angles );
}
}

float ComputeShadowControlBox3D( CPhysicsObjectBox3D *pObject, const hlshadowcontrol_params_t &params,
	float secondsToArrival, float dt, Vector *pLastImpulse )
{
	// Resample so the object arrives at the requested time.
	float fraction = 1.0f;
	if ( secondsToArrival > 0 )
	{
		fraction = dt / secondsToArrival;
		if ( fraction > 1 )
			fraction = 1;
	}
	secondsToArrival -= dt;
	if ( secondsToArrival < 0 )
		secondsToArrival = 0;
	if ( fraction <= 0 || dt <= 0 || !pObject->IsMoveable() )
		return secondsToArrival;

	Vector position;
	QAngle angles;
	pObject->GetPosition( &position, &angles );
	Vector deltaPosition = params.targetPosition - position;
	if ( params.teleportDistance > 0 && deltaPosition.LengthSqr() > params.teleportDistance * params.teleportDistance )
	{
		pObject->TeleportTo( params.targetPosition, params.targetRotation );
		deltaPosition.Init();
		angles = params.targetRotation;
	}

	float invDt = 1.0f / dt;
	Vector linear, angular;
	pObject->GetWorldVelocity( &linear, &angular );
	ComputeController( linear, deltaPosition, params.maxSpeed, params.maxDampSpeed, fraction * invDt, params.dampFactor, pLastImpulse );

	Vector deltaAngles = RotationError( ToB3( angles ), ToB3( params.targetRotation ) );
	ComputeController( angular, deltaAngles, DEG2RAD( params.maxAngular ), DEG2RAD( params.maxDampAngular ),
		fraction * invDt, params.dampFactor, NULL );

	pObject->SetWorldVelocity( linear, angular );
	return secondsToArrival;
}

//-----------------------------------------------------------------------------
// Shadow controller
//-----------------------------------------------------------------------------
CShadowControllerBox3D::CShadowControllerBox3D( CPhysicsObjectBox3D *pObject, bool allowTranslation, bool allowRotation )
	: m_pObject( pObject ), m_maxSpeed( 0.0f ), m_maxAngularSpeed( 0.0f ), m_teleportDistance( 0.0f ),
	  m_secondsToArrival( 0.0f ), m_dampFactor( 1.0f ), m_savedMass( pObject->GetMass() ),
	  m_allowTranslation( allowTranslation ), m_allowRotation( allowRotation ), m_physicallyControlled( false ),
	  m_enabled( false ), m_tempDisableGravity( false )
{
	m_lastImpulse.Init();
	pObject->GetPosition( &m_targetPosition, &m_targetAngles );

	// Attach exactly as IVP's CShadowController::AttachObject does.
	m_savedCallbackFlags = pObject->GetCallbackFlags();
	unsigned short flags = m_savedCallbackFlags | CALLBACK_SHADOW_COLLISION;
	flags &= ~( CALLBACK_GLOBAL_FRICTION | CALLBACK_GLOBAL_COLLIDE_STATIC );
	pObject->SetCallbackFlags( flags );
	pObject->EnableDrag( false );
	if ( !allowTranslation )
	{
		pObject->SetMass( VPHYSICS_MAX_MASS );
		pObject->EnableGravity( false );
	}
}

CShadowControllerBox3D::~CShadowControllerBox3D()
{
	m_pObject->SetCallbackFlags( m_savedCallbackFlags );
	m_pObject->EnableDrag( true );
	m_pObject->EnableGravity( true );
	if ( !m_allowTranslation )
		m_pObject->SetMass( m_savedMass );
}

void CShadowControllerBox3D::Update( const Vector &position, const QAngle &angles, float timeOffset )
{
	bool changed = position != m_targetPosition || angles != m_targetAngles;
	m_targetPosition = position;
	m_targetAngles = angles;
	m_secondsToArrival = timeOffset < 0 ? 0 : timeOffset;
	m_enabled = true;
	if ( changed )
		m_pObject->Wake();
}

void CShadowControllerBox3D::StepUp( float height )
{
	TeleportUp( m_pObject, height );
}

void CShadowControllerBox3D::Simulate( float dt )
{
	if ( !m_enabled )
		return;
	hlshadowcontrol_params_t params;
	params.targetPosition = m_targetPosition;
	params.targetRotation = m_targetAngles;
	params.maxSpeed = m_maxSpeed;
	params.maxDampSpeed = m_maxSpeed;
	params.maxAngular = m_maxAngularSpeed;
	params.maxDampAngular = m_maxAngularSpeed;
	params.dampFactor = m_dampFactor;
	params.teleportDistance = m_teleportDistance;
	m_secondsToArrival = ComputeShadowControlBox3D( m_pObject, params, m_secondsToArrival, dt, &m_lastImpulse );
}

//-----------------------------------------------------------------------------
// Player controller
//-----------------------------------------------------------------------------
CPlayerControllerBox3D::CPlayerControllerBox3D( CPhysicsObjectBox3D *pObject )
	: m_pObject( pObject ), m_pHandler( NULL ), m_secondsToArrival( 0.0f ), m_pushMassLimit( 1e4f ),
	  m_pushSpeedLimit( 1e4f ), m_enabled( false )
{
	pObject->GetPosition( &m_targetPosition, NULL );
	m_targetVelocity.Init();
	m_lastImpulse.Init();
	m_maxVelocity.Init( 1e4f, 1e4f, 1e4f );
	pObject->SetPlayerController( this );
}

void CPlayerControllerBox3D::Update( const Vector &position, const Vector &velocity, float secondsToArrival, bool onground, IPhysicsObject *ground )
{
	m_targetPosition = position;
	m_targetVelocity = velocity;
	m_secondsToArrival = secondsToArrival < 0 ? 0 : secondsToArrival;
	m_enabled = true;
	m_pObject->Wake();
}

bool CPlayerControllerBox3D::IsInContact( void )
{
	CFrictionSnapshotBox3D snapshot( m_pObject );
	for ( ; snapshot.IsValid(); snapshot.NextFrictionData() )
	{
		CPhysicsObjectBox3D *pOther = ToBox3D( snapshot.GetObject( 1 ) );
		// In contact with something physically simulated (not static, pinned,
		// or itself game-controlled).
		if ( pOther && pOther->IsMoveable() && !pOther->GetShadow() && !pOther->GetPlayerController() )
			return true;
	}
	return false;
}

void CPlayerControllerBox3D::SetObject( IPhysicsObject *pObject )
{
	if ( pObject == m_pObject )
		return;
	m_pObject->SetPlayerController( NULL );
	m_pObject = ToBox3D( pObject );
	m_pObject->SetPlayerController( this );
}

int CPlayerControllerBox3D::GetShadowPosition( Vector *position, QAngle *angles )
{
	m_pObject->GetPosition( position, angles );
	return 1;
}

void CPlayerControllerBox3D::StepUp( float height )
{
	TeleportUp( m_pObject, height );
}

void CPlayerControllerBox3D::GetShadowVelocity( Vector *velocity )
{
	m_pObject->GetWorldVelocity( velocity, NULL );
}

IPhysicsObject *CPlayerControllerBox3D::GetObject()
{
	return m_pObject;
}

void CPlayerControllerBox3D::Simulate( float dt )
{
	if ( !m_enabled )
		return;
	QAngle angles;
	m_pObject->GetPosition( NULL, &angles );
	hlshadowcontrol_params_t params;
	params.targetPosition = m_targetPosition + m_targetVelocity * m_secondsToArrival;
	params.targetRotation = angles;
	params.maxSpeed = m_maxVelocity.Length();
	params.maxDampSpeed = params.maxSpeed;
	params.maxAngular = 0.0f;
	params.maxDampAngular = 0.0f;
	params.dampFactor = 1.0f;
	params.teleportDistance = 0.0f;
	m_secondsToArrival = ComputeShadowControlBox3D( m_pObject, params, m_secondsToArrival, dt, &m_lastImpulse );
}

//-----------------------------------------------------------------------------
// Motion controller
//-----------------------------------------------------------------------------
void CMotionControllerBox3D::Simulate( float dt )
{
	if ( !m_pHandler )
		return;
	// The handler may attach or detach objects; iterate a snapshot.
	CUtlVector<IPhysicsObject *> objects;
	objects.CopyArray( m_objects.Base(), m_objects.Count() );
	for ( int i = 0; i < objects.Count(); i++ )
	{
		CPhysicsObjectBox3D *pObject = ToBox3D( objects[i] );
		if ( !pObject->IsMoveable() || pObject->IsAsleep() )
			continue;

		Vector linear( 0, 0, 0 );
		AngularImpulse angular( 0, 0, 0 );
		IMotionEvent::simresult_e result = m_pHandler->Simulate( this, pObject, dt, linear, angular );
		if ( result == IMotionEvent::SIM_NOTHING )
			continue;

		// Angular terms are object-local in every mode (IVP adds them to the
		// core-space rotation speed); linear terms are local or world by mode.
		bool local = result == IMotionEvent::SIM_LOCAL_ACCELERATION || result == IMotionEvent::SIM_LOCAL_FORCE;
		bool force = result == IMotionEvent::SIM_LOCAL_FORCE || result == IMotionEvent::SIM_GLOBAL_FORCE;
		Vector worldLinear = linear;
		if ( local )
			pObject->LocalToWorldVector( &worldLinear, linear );
		Vector localAngular( DEG2RAD( angular.x ), DEG2RAD( angular.y ), DEG2RAD( angular.z ) );
		if ( force )
		{
			worldLinear *= pObject->GetInvMass();
			Vector invInertia = pObject->GetInvInertia();
			localAngular.x *= invInertia.x;
			localAngular.y *= invInertia.y;
			localAngular.z *= invInertia.z;
		}
		Vector worldAngular;
		pObject->LocalToWorldVector( &worldAngular, localAngular );

		Vector velocity, angularVelocity;
		pObject->GetWorldVelocity( &velocity, &angularVelocity );
		pObject->SetWorldVelocity( velocity + worldLinear * dt, angularVelocity + worldAngular * dt );
	}
}

//-----------------------------------------------------------------------------
// Constraint
//-----------------------------------------------------------------------------
CConstraintBox3D::~CConstraintBox3D()
{
	if ( b3Joint_IsValid( m_joint ) )
		b3DestroyJoint( m_joint, true );
}

//-----------------------------------------------------------------------------
// Friction snapshot
//-----------------------------------------------------------------------------
CFrictionSnapshotBox3D::CFrictionSnapshotBox3D( CPhysicsObjectBox3D *pObject ) : m_pObject( pObject ), m_index( 0 )
{
	b3BodyId body = pObject->GetBody();
	int capacity = b3Body_GetContactCapacity( body );
	if ( capacity <= 0 )
		return;
	CUtlVector<b3ContactData> contacts;
	contacts.SetCount( capacity );
	int count = b3Body_GetContactData( body, contacts.Base(), capacity );
	float stepTime = pObject->GetEnvironment()->GetStepTime();
	for ( int c = 0; c < count; c++ )
	{
		const b3ContactData &data = contacts[c];
		CPhysicsObjectBox3D *pA = (CPhysicsObjectBox3D *)b3Shape_GetUserData( data.shapeIdA );
		CPhysicsObjectBox3D *pB = (CPhysicsObjectBox3D *)b3Shape_GetUserData( data.shapeIdB );
		bool isA = pA == pObject;
		CPhysicsObjectBox3D *pOther = isA ? pB : pA;
		for ( int m = 0; m < data.manifoldCount; m++ )
		{
			const b3Manifold &manifold = data.manifolds[m];
			if ( manifold.pointCount <= 0 )
				continue;
			Entry_t entry;
			entry.pOther = pOther;
			entry.normal = FromB3( manifold.normal );
			if ( !isA )
				entry.normal = -entry.normal;
			// Anchor A is relative to body A's center of mass, in world space.
			b3BodyId bodyA = b3Shape_GetBody( data.shapeIdA );
			Vector centerA = FromB3( b3Body_GetWorldCenter( bodyA ) );
			Vector point( 0, 0, 0 );
			float impulse = 0.0f;
			for ( int p = 0; p < manifold.pointCount; p++ )
			{
				point += centerA + FromB3( manifold.points[p].anchorA );
				impulse += manifold.points[p].totalNormalImpulse;
			}
			entry.point = point / (float)manifold.pointCount;
			entry.normalForce = stepTime > 0 ? impulse / stepTime : 0.0f;
			entry.friction = sqrtf( pObject->GetFriction() * ( pOther ? pOther->GetFriction() : 1.0f ) );
			m_entries.AddToTail( entry );
		}
	}
}

IPhysicsObject *CFrictionSnapshotBox3D::GetObject( int index )
{
	if ( !IsValid() )
		return NULL;
	return index == 0 ? (IPhysicsObject *)m_pObject : (IPhysicsObject *)Current().pOther;
}

int CFrictionSnapshotBox3D::GetMaterial( int index )
{
	IPhysicsObject *pObject = GetObject( index );
	return pObject ? pObject->GetMaterialIndex() : 0;
}
