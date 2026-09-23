//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D shadow, player and motion controllers, constraints and
//          friction snapshots (RFC 0004 B5/C3/C4).
//
//=============================================================================//
#include "physics_controllers.h"

#include <math.h>
#include <string.h>

#include "box3d/box3d.h"
#include "box3d_convert.h"
#include "physics_environment.h"
#include "physics_object.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
// IVP's reserved shadow surface ($MATERIAL_INDEX_SHADOW).
const int kMaterialIndexShadow = 0xF000;

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
	  m_savedInertia( pObject->GetInertia() ), m_savedMaterialIndex( pObject->GetMaterialIndex() ),
	  m_allowTranslation( allowTranslation ), m_allowRotation( allowRotation ), m_physicallyControlled( false ),
	  m_enabled( false )
{
	m_lastImpulse.Init();
	pObject->GetPosition( &m_targetPosition, &m_targetAngles );

	// Attach exactly as IVP's CShadowController::AttachObject does.
	pObject->GetDamping( NULL, &m_savedRotDamping );
	UseShadowMaterial( true );
	float rotDamping = 100.0f;
	pObject->SetDamping( NULL, &rotDamping );
	if ( !allowRotation )
		pObject->SetInertia( Vector( 1e14f, 1e14f, 1e14f ) );
	if ( !allowTranslation )
	{
		pObject->SetMass( VPHYSICS_MAX_MASS );
		pObject->EnableGravity( false );
	}
	m_savedCallbackFlags = pObject->GetCallbackFlags();
	unsigned short flags = m_savedCallbackFlags | CALLBACK_SHADOW_COLLISION;
	flags &= ~( CALLBACK_GLOBAL_FRICTION | CALLBACK_GLOBAL_COLLIDE_STATIC );
	pObject->SetCallbackFlags( flags );
	pObject->EnableDrag( false );
}

CShadowControllerBox3D::CShadowControllerBox3D( CPhysicsObjectBox3D *pObject, const State_t &state )
	: m_pObject( pObject ), m_targetPosition( state.targetPosition ), m_targetAngles( state.targetAngles ),
	  m_lastImpulse( state.lastImpulse ), m_maxSpeed( state.maxSpeed ), m_maxAngularSpeed( state.maxAngularSpeed ),
	  m_teleportDistance( state.teleportDistance ), m_secondsToArrival( state.secondsToArrival ), m_dampFactor( state.dampFactor ),
	  m_savedCallbackFlags( (unsigned short)state.savedCallbackFlags ), m_savedMass( state.savedMass ),
	  m_savedRotDamping( state.savedRotDamping ), m_savedInertia( state.savedInertia ), m_savedMaterialIndex( state.savedMaterialIndex ),
	  m_allowTranslation( state.allowTranslation ), m_allowRotation( state.allowRotation ),
	  m_physicallyControlled( state.physicallyControlled ), m_enabled( state.enabled )
{
}

void CShadowControllerBox3D::WriteState( State_t &state ) const
{
	memset( &state, 0, sizeof( state ) );
	state.targetPosition = m_targetPosition;
	state.targetAngles = m_targetAngles;
	state.lastImpulse = m_lastImpulse;
	state.maxSpeed = m_maxSpeed;
	state.maxAngularSpeed = m_maxAngularSpeed;
	state.teleportDistance = m_teleportDistance;
	state.secondsToArrival = m_secondsToArrival;
	state.dampFactor = m_dampFactor;
	state.savedCallbackFlags = m_savedCallbackFlags;
	state.savedMass = m_savedMass;
	state.savedRotDamping = m_savedRotDamping;
	state.savedInertia = m_savedInertia;
	state.savedMaterialIndex = m_savedMaterialIndex;
	state.allowTranslation = m_allowTranslation;
	state.allowRotation = m_allowRotation;
	state.physicallyControlled = m_physicallyControlled;
	state.enabled = m_enabled;
}

CShadowControllerBox3D::~CShadowControllerBox3D()
{
	// IVP's DetachObject: restore everything, unless the object is being
	// deleted anyway.
	if ( m_pObject->GetCallbackFlags() & CALLBACK_MARKED_FOR_DELETE )
		return;
	m_pObject->SetDamping( NULL, &m_savedRotDamping );
	m_pObject->SetMass( m_savedMass );
	m_pObject->SetCallbackFlags( m_savedCallbackFlags );
	m_pObject->EnableDrag( true );
	m_pObject->EnableGravity( true );
	UseShadowMaterial( false );
	m_pObject->SetInertia( m_savedInertia );
}

void CShadowControllerBox3D::UseShadowMaterial( bool bUseShadowMaterial )
{
	// The reserved shadow surface ($MATERIAL_INDEX_SHADOW) while shadowed.
	int current = m_pObject->GetMaterialIndex();
	int target = bUseShadowMaterial ? kMaterialIndexShadow : m_savedMaterialIndex;
	if ( target != current )
	{
		int saved = m_savedMaterialIndex;
		m_pObject->SetMaterialIndex( target );
		m_savedMaterialIndex = saved;
	}
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
	: m_pObject( pObject ), m_pGround( NULL ), m_pHandler( NULL ), m_savedRotDamping( 0.0f ), m_maxDeltaPosition( 24.0f ),
	  m_dampFactor( 1.0f ), m_secondsToArrival( 0.0f ), m_pushMassLimit( VPHYSICS_MAX_MASS ), m_pushSpeedLimit( 1e4f ),
	  m_enabled( false ), m_forceTeleport( false ), m_updatedSinceLast( false )
{
	m_targetPosition.Init();
	m_groundPosition.Init();
	m_maxSpeed.Init();
	m_currentSpeed.Init();
	m_lastImpulse.Init();
	AttachObject();
}

CPlayerControllerBox3D::~CPlayerControllerBox3D()
{
	DetachObject();
}

// As IVP: no drag, heavy rotational damping, and the player-controller flag.
void CPlayerControllerBox3D::AttachObject()
{
	m_pObject->EnableDrag( false );
	float speed;
	m_pObject->GetDamping( &speed, &m_savedRotDamping );
	float rot = 100.0f;
	m_pObject->SetDamping( NULL, &rot );
	m_pObject->SetCallbackFlags( m_pObject->GetCallbackFlags() | CALLBACK_IS_PLAYER_CONTROLLER );
	m_pObject->SetPlayerController( this );
}

void CPlayerControllerBox3D::DetachObject()
{
	if ( !m_pObject )
		return;
	m_pObject->SetDamping( NULL, &m_savedRotDamping );
	m_pObject->SetCallbackFlags( m_pObject->GetCallbackFlags() & ~CALLBACK_IS_PLAYER_CONTROLLER );
	m_pObject->SetPlayerController( NULL );
	m_pObject = NULL;
	m_pGround = NULL;
}

void CPlayerControllerBox3D::ObjectDestroyed( IPhysicsObject *pObject )
{
	if ( pObject == m_pGround )
		m_pGround = NULL;
}

// Velocity of the ground under the target point.
Vector CPlayerControllerBox3D::GroundVelocity() const
{
	Vector velocity( 0, 0, 0 );
	if ( m_pGround )
	{
		Vector world;
		m_pGround->LocalToWorld( &world, m_groundPosition );
		m_pGround->GetVelocityAtPoint( world, &velocity );
	}
	return velocity;
}

void CPlayerControllerBox3D::Update( const Vector &position, const Vector &velocity, float secondsToArrival, bool onground, IPhysicsObject *ground )
{
	m_updatedSinceLast = true;
	// Nothing changed: keep the plan (IVP compares in meters, 1e-6 m^2).
	const float kSameSq = 1e-6f / ( 0.0254f * 0.0254f );
	if ( velocity.DistToSqr( m_currentSpeed ) < kSameSq && position.DistToSqr( m_targetPosition ) < kSameSq )
		return;

	m_targetPosition = position;
	m_secondsToArrival = secondsToArrival < 0 ? 0 : secondsToArrival;
	m_currentSpeed = velocity;
	m_pObject->Wake();

	m_enabled = true;
	if ( velocity.LengthSqr() <= 0.1f )
	{
		// No input velocity: go where physics takes the object.
		m_enabled = false;
		ground = NULL;
	}
	else
	{
		MaxSpeed( velocity );
	}
	m_pGround = ground ? ToBox3D( ground ) : NULL;
	if ( m_pGround )
		m_pGround->WorldToLocal( &m_groundPosition, m_targetPosition );
}

// IVP removes the part of the requested velocity the object already has
// along it. The product dot * length is formed in meters, as IVP does.
void CPlayerControllerBox3D::MaxSpeed( const Vector &maxVelocity )
{
	const float kMeters = 0.0254f;
	Vector requested = maxVelocity * kMeters;
	Vector available = requested;
	Vector direction = requested;
	float length = VectorNormalize( direction );
	Vector current;
	m_pObject->GetVelocity( &current, NULL );
	float dot = DotProduct( direction, current * kMeters );
	if ( dot > 0 )
		available -= direction * ( dot * length );
	m_maxSpeed.Init( fabsf( available.x ), fabsf( available.y ), fabsf( available.z ) );
	m_maxSpeed /= kMeters;
}

bool CPlayerControllerBox3D::IsInContact( void )
{
	if ( !m_pObject->IsCollisionEnabled() )
		return false;
	CFrictionSnapshotBox3D snapshot( m_pObject );
	for ( ; snapshot.IsValid(); snapshot.NextFrictionData() )
	{
		CPhysicsObjectBox3D *pOther = ToBox3D( snapshot.GetObject( 1 ) );
		// In contact with something physically simulated: not static,
		// pinned, or itself controlled by the game.
		if ( pOther && pOther->IsCollisionEnabled() && !pOther->IsStatic() && pOther->IsMotionEnabled() &&
			!pOther->IsControlledByGame() )
			return true;
	}
	return false;
}

void CPlayerControllerBox3D::SetObject( IPhysicsObject *pObject )
{
	CPhysicsObjectBox3D *pBox = ToBox3D( pObject );
	if ( pBox == m_pObject )
		return;
	DetachObject();
	m_pObject = pBox;
	AttachObject();
}

int CPlayerControllerBox3D::GetShadowPosition( Vector *position, QAngle *angles )
{
	m_pObject->GetPosition( position, angles );
	return 1;
}

void CPlayerControllerBox3D::StepUp( float height )
{
	if ( height == 0.0f )
		return;
	TeleportUp( m_pObject, height );
}

void CPlayerControllerBox3D::GetShadowVelocity( Vector *velocity )
{
	if ( !velocity )
		return;
	m_pObject->GetVelocity( velocity, NULL );
	*velocity -= GroundVelocity();
}

IPhysicsObject *CPlayerControllerBox3D::GetObject()
{
	return m_pObject;
}

bool CPlayerControllerBox3D::TryTeleportObject()
{
	if ( m_pHandler && !m_forceTeleport && !m_pHandler->ShouldMoveTo( m_pObject, m_targetPosition ) )
		return false;
	QAngle angles;
	m_pObject->GetPosition( NULL, &angles );
	m_pObject->SetPosition( m_targetPosition, angles, true );
	m_forceTeleport = false;
	return true;
}

namespace
{
// IVP's per-axis clamped controller (ComputeController, vector limit form).
Vector ComputePlayerImpulse( Vector &speed, const Vector &delta, const Vector &maxSpeed, float scaleDelta, float damping )
{
	const float kZeroSpeedSq = 1e-6f / ( 0.0254f * 0.0254f );
	if ( speed.LengthSqr() < kZeroSpeedSq )
		speed.Init();
	Vector acceleration = delta * scaleDelta - speed * damping;
	for ( int i = 2; i >= 0; i-- )
	{
		if ( fabsf( acceleration[i] ) >= maxSpeed[i] )
			acceleration[i] = acceleration[i] < 0 ? -maxSpeed[i] : maxSpeed[i];
	}
	speed += acceleration;
	return acceleration;
}

// The contact planes the player is pushing into harder than allowed.
class CNormalList
{
public:
	CNormalList() : m_count( 0 ) {}
	void AddNormal( const Vector &normal )
	{
		if ( m_count == kMaxNormals )
			return;
		for ( int i = m_count; --i >= 0; )
		{
			if ( DotProduct( m_normals[i], normal ) > 0.99f )
				return;
		}
		m_normals[m_count++] = normal;
	}
	Vector ClampVector( const Vector &in, float limitVel ) const
	{
		if ( m_count > 2 )
		{
			for ( int i = 0; i < m_count; i++ )
			{
				if ( DotProduct( in, m_normals[i] ) > 0 )
					return vec3_origin;
			}
		}
		else if ( m_count == 2 )
		{
			Vector crease = CrossProduct( m_normals[0], m_normals[1] );
			return crease * DotProduct( in, crease );
		}
		else if ( m_count == 1 )
		{
			float dot = DotProduct( in, m_normals[0] );
			if ( dot > limitVel )
				return in + m_normals[0] * ( limitVel - dot );
		}
		return in;
	}

private:
	static const int kMaxNormals = 8;
	Vector m_normals[kMaxNormals];
	int m_count;
};
}

// IVP's CPlayerController::do_simulation_controller, in inches.
void CPlayerControllerBox3D::Simulate( float dt )
{
	if ( !m_enabled || !m_pObject || !m_pObject->IsMoveable() || m_pObject->IsAsleep() || dt <= 0 )
		return;

	Vector speed;
	m_pObject->GetWorldVelocity( &speed, NULL );
	Vector baseVelocity( 0, 0, 0 );
	if ( m_pGround )
	{
		// The target rides on the ground; work relative to its surface.
		m_pGround->LocalToWorld( &m_targetPosition, m_groundPosition );
		baseVelocity = GroundVelocity();
		speed -= baseVelocity;
	}

	Vector position;
	m_pObject->GetPosition( &position, NULL );
	Vector delta = m_targetPosition - position;
	if ( m_forceTeleport || delta.LengthSqr() > m_maxDeltaPosition * m_maxDeltaPosition )
	{
		if ( TryTeleportObject() )
			return;
	}

	float fraction = 1.0f;
	if ( m_secondsToArrival > 0 )
		fraction = MIN( dt / m_secondsToArrival, 1.0f );
	if ( !m_updatedSinceLast )
	{
		// No game update since the last step: limit to the last known good
		// impulse, and keep it.
		float length = m_lastImpulse.Length();
		ComputePlayerImpulse( speed, delta, Vector( length, length, length ), fraction / dt, m_dampFactor );
	}
	else
	{
		m_lastImpulse = ComputePlayerImpulse( speed, delta, m_maxSpeed, fraction / dt, m_dampFactor );
	}
	speed += baseVelocity;
	m_updatedSinceLast = false;

	// Don't push into immovable, too heavy, or too fast-pushed contacts.
	Vector pushVelocity = speed;
	bool onGround = false;
	float invMass = m_pObject->GetInvMass();
	float limitVel = m_pushSpeedLimit;
	CNormalList normals;
	for ( CFrictionSnapshotBox3D snapshot( m_pObject ); snapshot.IsValid(); snapshot.NextFrictionData() )
	{
		Vector normal;
		snapshot.GetSurfaceNormal( normal );
		if ( normal.z < -0.7f )
			onGround = true;
		if ( normal.z > -0.99f )
		{
			IPhysicsObject *pOther = snapshot.GetObject( 1 );
			if ( !pOther || !pOther->IsMoveable() || pOther->GetMass() > m_pushMassLimit )
				limitVel = 0.0f;
			float pushTotal = DotProduct( pushVelocity, normal ) + snapshot.GetNormalForce() * invMass;
			if ( pushTotal > limitVel )
				normals.AddNormal( normal );
		}
	}
	Vector limit = normals.ClampVector( pushVelocity, limitVel ) - pushVelocity;
	speed += limit;
	m_lastImpulse += limit;

	if ( onGround )
	{
		// Moving down: press down with full gravity and no more.
		Vector gravity;
		m_pObject->GetEnvironment()->GetGravity( &gravity );
		float gravityDt = gravity.Length() * dt;
		if ( m_lastImpulse.z <= 0 )
		{
			float change = -gravityDt - m_lastImpulse.z;
			speed.z += change;
			m_lastImpulse.z += change;
		}
	}
	Vector current, angular;
	m_pObject->GetWorldVelocity( &current, &angular );
	m_pObject->SetWorldVelocity( speed, angular );

	m_secondsToArrival = MAX( m_secondsToArrival - dt, 0.0f );
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
			// IVP's product rule (see the environment's mixing callbacks).
			entry.friction = clamp( pObject->GetFriction() * ( pOther ? pOther->GetFriction() : 1.0f ), 0.0f, 1.0f );
			m_entries.AddToTail( entry );
		}
	}
}

void CFrictionSnapshotBox3D::MarkContactForDelete()
{
	CPhysicsObjectBox3D *pOther = IsValid() ? Current().pOther : NULL;
	if ( pOther && pOther != m_pObject && m_marked.Find( pOther ) == m_marked.InvalidIndex() )
		m_marked.AddToTail( pOther );
}

// Deletes the contacts with every marked partner; like IVP's, the snapshot
// is exhausted afterwards.
void CFrictionSnapshotBox3D::DeleteAllMarkedContacts( bool wakeObjects )
{
	for ( int i = 0; i < m_marked.Count(); i++ )
		m_pObject->GetEnvironment()->DeleteContactPair( m_pObject, m_marked[i], wakeObjects );
	if ( m_marked.Count() )
		m_index = m_entries.Count();
	m_marked.RemoveAll();
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
