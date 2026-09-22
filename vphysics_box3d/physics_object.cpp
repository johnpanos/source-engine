//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics object backed by a Box3D body (RFC 0004 B2/B5).
//
//=============================================================================//
#include "physics_object.h"

#include <string.h>

#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d_convert.h"
#include "bspflags.h"
#include "physics_collision.h"
#include "physics_controllers.h"
#include "physics_environment.h"
#include "physics_material.h"
#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
// IVP's default callback set for a new object (vphysics/physics_object.cpp).
const unsigned short kDefaultCallbacks = CALLBACK_GLOBAL_COLLISION | CALLBACK_GLOBAL_FRICTION |
	CALLBACK_FLUID_TOUCH | CALLBACK_GLOBAL_TOUCH | CALLBACK_GLOBAL_COLLIDE_STATIC | CALLBACK_DO_FLUID_SIMULATION;
const int kMaxShapes = 256;

b3WorldTransform BodyTransform( b3BodyId body )
{
	return b3Body_GetTransform( body );
}
}

CPhysicsObjectBox3D::CPhysicsObjectBox3D( CPhysicsEnvironmentBox3D *pEnv, const CPhysCollide *pCollide, float sphereRadius,
	int materialIndex, const Vector &position, const QAngle &angles, const objectparams_t *pParams, bool isStatic )
	: m_pEnv( pEnv ), m_pCollide( pCollide ), m_body( b3_nullBodyId ), m_pShadow( NULL ), m_pPlayerController( NULL ),
	  m_pGameData( pParams ? pParams->pGameData : NULL ), m_mass( pParams ? pParams->mass : 1.0f ),
	  m_inertiaScale( pParams && pParams->inertia > 0.0f ? pParams->inertia : 1.0f ), m_sphereRadius( sphereRadius ),
	  m_linearDamping( pParams ? pParams->damping : 0.0f ), m_angularDamping( pParams ? pParams->rotdamping : 0.0f ),
	  m_friction( 0.8f ), m_restitution( 0.0f ), m_materialIndex( materialIndex ), m_hingeAxis( -1 ),
	  m_contents( CONTENTS_SOLID ), m_callbackFlags( kDefaultCallbacks ), m_gameFlags( 0 ), m_gameIndex( 0 ),
	  m_isStatic( isStatic ), m_isTrigger( false ), m_collisionEnabled( pParams ? pParams->enableCollisions : true ),
	  m_gravityEnabled( !isStatic ), m_dragEnabled( !isStatic ), m_motionEnabled( !isStatic ), m_wasAwake( false ), m_reportPreStep( false )
{
	m_preStepLinear.Init();
	m_preStepAngular.Init();
	V_strncpy( m_name, pParams && pParams->pName ? pParams->pName : "box3d_object", sizeof( m_name ) );
	m_inertia.Init( 1, 1, 1 );

	const CPhysCollideBox3D *pBox = pCollide ? ToBox3D( pCollide ) : NULL;
	m_massCenter = pBox ? pBox->massCenter : vec3_origin;
	if ( pParams && pParams->massCenterOverride )
		m_massCenter = *pParams->massCenterOverride;

	surfacedata_t *pSurface = g_SurfaceDatabase.GetSurfaceData( materialIndex );
	if ( pSurface )
	{
		m_friction = pSurface->physics.friction;
		m_restitution = pSurface->physics.elasticity;
	}

	b3BodyDef def = b3DefaultBodyDef();
	def.type = isStatic ? b3_staticBody : b3_dynamicBody;
	def.position = ToB3( position );
	def.rotation = ToB3( angles );
	def.linearDamping = m_linearDamping;
	def.angularDamping = m_angularDamping;
	def.userData = this;
	def.name = m_name;
	def.isAwake = false;
	m_body = b3CreateBody( pEnv->GetWorld(), &def );

	CreateShapes();
	ApplyMassProperties();
	ApplyFilter();
	m_wasAwake = b3Body_IsAwake( m_body );
}

CPhysicsObjectBox3D::~CPhysicsObjectBox3D()
{
	delete m_pShadow;
	m_pShadow = NULL;
	if ( b3Body_IsValid( m_body ) )
		b3DestroyBody( m_body );
}

void CPhysicsObjectBox3D::CreateShapes()
{
	b3ShapeDef def = b3DefaultShapeDef();
	def.userData = this;
	def.enableCustomFiltering = true;
	def.enableContactEvents = true;
	def.updateBodyMass = false;
	def.density = 1.0f;
	def.baseMaterial.friction = m_friction;
	def.baseMaterial.restitution = m_restitution;
	def.baseMaterial.userMaterialId = (uint64_t)m_materialIndex;

	if ( m_sphereRadius > 0.0f )
	{
		b3Sphere sphere;
		sphere.center = b3Vec3_zero;
		sphere.radius = m_sphereRadius;
		b3CreateSphereShape( m_body, &def, &sphere );
		return;
	}

	const CPhysCollideBox3D *pBox = m_pCollide ? ToBox3D( m_pCollide ) : NULL;
	for ( int i = 0; pBox && i < pBox->convexes.Count(); i++ )
	{
		const CPhysConvexBox3D *pConvex = pBox->convexes[i];
		for ( int h = 0; h < pConvex->hulls.Count(); h++ )
			b3CreateHullShape( m_body, &def, pConvex->hulls[h] );
	}
}

float CPhysicsObjectBox3D::ComputeShapeVolume() const
{
	if ( m_sphereRadius > 0.0f )
		return ( 4.0f / 3.0f ) * M_PI_F * m_sphereRadius * m_sphereRadius * m_sphereRadius;
	const CPhysCollideBox3D *pBox = m_pCollide ? ToBox3D( m_pCollide ) : NULL;
	return pBox ? pBox->volume : 0.0f;
}

void CPhysicsObjectBox3D::ApplyMassProperties()
{
	if ( m_isStatic || b3Body_GetType( m_body ) != b3_dynamicBody )
		return;

	float volume = ComputeShapeVolume();
	b3ShapeId shapes[kMaxShapes];
	int shapeCount = b3Body_GetShapes( m_body, shapes, kMaxShapes );
	b3MassData mass;
	if ( volume > 0.0f && shapeCount > 0 )
	{
		// Uniform density that yields the authored mass, so Box3D computes a
		// consistent inertia tensor for the actual geometry.
		float density = m_mass / volume;
		for ( int i = 0; i < shapeCount; i++ )
			b3Shape_SetDensity( shapes[i], density, false );
		b3Body_ApplyMassFromShapes( m_body );
		mass = b3Body_GetMassData( m_body );
		// Pieces covering a large convex overlap inside it, so the shape sum
		// can exceed the authored mass; keep the inertia consistent with it.
		if ( mass.mass > 0.0f )
		{
			float correction = m_mass / mass.mass;
			mass.inertia.cx = b3MulSV( correction, mass.inertia.cx );
			mass.inertia.cy = b3MulSV( correction, mass.inertia.cy );
			mass.inertia.cz = b3MulSV( correction, mass.inertia.cz );
		}
	}
	else
	{
		mass.center = b3Vec3_zero;
		float inertia = m_mass * 16.0f;
		mass.inertia.cx = { inertia, 0, 0 };
		mass.inertia.cy = { 0, inertia, 0 };
		mass.inertia.cz = { 0, 0, inertia };
	}
	mass.mass = m_mass;
	mass.center = ToB3( m_massCenter );

	float scale = m_inertiaScale;
	if ( m_pShadow && !m_pShadow->AllowsRotation() )
		scale *= 1e6f;	// IVP pins a non-rotating shadow with 1e14 inertia
	mass.inertia.cx = b3MulSV( scale, mass.inertia.cx );
	mass.inertia.cy = b3MulSV( scale, mass.inertia.cy );
	mass.inertia.cz = b3MulSV( scale, mass.inertia.cz );
	b3Body_SetMassData( m_body, mass );
	m_inertia.Init( mass.inertia.cx.x, mass.inertia.cy.y, mass.inertia.cz.z );
}

void CPhysicsObjectBox3D::ApplyBodyType()
{
	if ( m_isStatic )
		return;
	b3BodyType desired = m_motionEnabled ? b3_dynamicBody : b3_kinematicBody;
	if ( b3Body_GetType( m_body ) == desired )
		return;
	b3Body_SetType( m_body, desired );
	if ( desired == b3_kinematicBody )
	{
		b3Body_SetLinearVelocity( m_body, b3Vec3_zero );
		b3Body_SetAngularVelocity( m_body, b3Vec3_zero );
	}
	else
	{
		ApplyMassProperties();
	}
	b3Body_SetGravityScale( m_body, m_gravityEnabled ? 1.0f : 0.0f );
}

void CPhysicsObjectBox3D::ApplyFilter()
{
	b3ShapeId shapes[kMaxShapes];
	int shapeCount = b3Body_GetShapes( m_body, shapes, kMaxShapes );
	for ( int i = 0; i < shapeCount; i++ )
	{
		b3Filter filter = b3Shape_GetFilter( shapes[i] );
		bool collides = m_collisionEnabled && !m_isTrigger;
		filter.categoryBits = collides ? B3_DEFAULT_CATEGORY_BITS : 0;
		filter.maskBits = collides ? B3_DEFAULT_MASK_BITS : 0;
		b3Shape_SetFilter( shapes[i], filter, true );
	}
	if ( !m_isStatic )
		b3Body_SetGravityScale( m_body, m_gravityEnabled ? 1.0f : 0.0f );
}

bool CPhysicsObjectBox3D::IsAsleep() const
{
	return !b3Body_IsAwake( m_body );
}

bool CPhysicsObjectBox3D::IsAttachedToConstraint( bool bExternalOnly ) const
{
	// IVP parity: CPhysicsObject::IsAttachedToConstraint only recognizes
	// IVP_CP_CONSTRAINTS controllers, and every Havok constraint registers at
	// IVP_CP_CONSTRAINTS_MIN, so IVP answers false for all of them. Game code
	// (CanResolvePenetrationWithNPC) is tuned against that answer.
	return false;
}

void CPhysicsObjectBox3D::EnableCollisions( bool enable )
{
	if ( m_collisionEnabled == enable )
		return;
	m_collisionEnabled = enable;
	ApplyFilter();
}

void CPhysicsObjectBox3D::EnableGravity( bool enable )
{
	m_gravityEnabled = enable;
	if ( !m_isStatic )
		b3Body_SetGravityScale( m_body, enable ? 1.0f : 0.0f );
}

void CPhysicsObjectBox3D::ApplyGravityScale( bool suppress )
{
	if ( !m_isStatic )
		b3Body_SetGravityScale( m_body, ( m_gravityEnabled && !suppress ) ? 1.0f : 0.0f );
}

void CPhysicsObjectBox3D::EnableMotion( bool enable )
{
	if ( m_isStatic || m_motionEnabled == enable )
		return;
	m_motionEnabled = enable;
	ApplyBodyType();
}

void CPhysicsObjectBox3D::Wake( void )
{
	if ( !m_isStatic )
		b3Body_SetAwake( m_body, true );
}

void CPhysicsObjectBox3D::Sleep( void )
{
	if ( !m_isStatic )
		b3Body_SetAwake( m_body, false );
}

void CPhysicsObjectBox3D::RecheckCollisionFilter( void )
{
	// Re-running the filter makes Box3D re-evaluate existing pairs, which
	// re-invokes the environment's custom filter (the game's collision rules).
	ApplyFilter();
}

void CPhysicsObjectBox3D::SetMass( float mass )
{
	m_mass = mass > 0.0f ? mass : 1.0f;
	ApplyMassProperties();
}

Vector CPhysicsObjectBox3D::GetInvInertia( void ) const
{
	if ( !IsMoveable() )
		return vec3_origin;
	return Vector( m_inertia.x > 0 ? 1.0f / m_inertia.x : 0.0f, m_inertia.y > 0 ? 1.0f / m_inertia.y : 0.0f,
		m_inertia.z > 0 ? 1.0f / m_inertia.z : 0.0f );
}

void CPhysicsObjectBox3D::SetInertia( const Vector &inertia )
{
	if ( m_isStatic )
		return;
	b3MassData mass = b3Body_GetMassData( m_body );
	mass.inertia.cx = { inertia.x, 0, 0 };
	mass.inertia.cy = { 0, inertia.y, 0 };
	mass.inertia.cz = { 0, 0, inertia.z };
	b3Body_SetMassData( m_body, mass );
	m_inertia = inertia;
}

void CPhysicsObjectBox3D::SetDamping( const float *speed, const float *rot )
{
	if ( speed )
		m_linearDamping = *speed;
	if ( rot )
		m_angularDamping = *rot;
	b3Body_SetLinearDamping( m_body, m_linearDamping );
	b3Body_SetAngularDamping( m_body, m_angularDamping );
}

void CPhysicsObjectBox3D::GetDamping( float *speed, float *rot ) const
{
	if ( speed )
		*speed = m_linearDamping;
	if ( rot )
		*rot = m_angularDamping;
}

void CPhysicsObjectBox3D::SetMaterialIndex( int materialIndex )
{
	m_materialIndex = materialIndex;
	surfacedata_t *pSurface = g_SurfaceDatabase.GetSurfaceData( materialIndex );
	if ( !pSurface )
		return;
	m_friction = pSurface->physics.friction;
	m_restitution = pSurface->physics.elasticity;
	b3ShapeId shapes[kMaxShapes];
	int shapeCount = b3Body_GetShapes( m_body, shapes, kMaxShapes );
	for ( int i = 0; i < shapeCount; i++ )
	{
		b3Shape_SetFriction( shapes[i], m_friction );
		b3Shape_SetRestitution( shapes[i], m_restitution );
	}
}

float CPhysicsObjectBox3D::GetEnergy() const
{
	if ( !IsMoveable() )
		return 0.0f;
	Vector linear, angular;
	GetWorldVelocity( &linear, &angular );
	Vector localAngular;
	WorldToLocalVector( &localAngular, angular );
	float rotational = m_inertia.x * localAngular.x * localAngular.x + m_inertia.y * localAngular.y * localAngular.y +
		m_inertia.z * localAngular.z * localAngular.z;
	return 0.5f * ( m_mass * linear.LengthSqr() + rotational );
}

//-----------------------------------------------------------------------------
// Transform
//-----------------------------------------------------------------------------
void CPhysicsObjectBox3D::TeleportTo( const Vector &position, const QAngle &angles )
{
	b3Body_SetTransform( m_body, ToB3( position ), ToB3( angles ) );
}

void CPhysicsObjectBox3D::SetPosition( const Vector &worldPosition, const QAngle &angles, bool isTeleport )
{
	TeleportTo( worldPosition, angles );
}

void CPhysicsObjectBox3D::SetPositionMatrix( const matrix3x4_t &matrix, bool isTeleport )
{
	Vector position;
	QAngle angles;
	MatrixAngles( matrix, angles, position );
	SetPosition( position, angles, isTeleport );
}

void CPhysicsObjectBox3D::GetPosition( Vector *worldPosition, QAngle *angles ) const
{
	b3WorldTransform xform = BodyTransform( m_body );
	if ( worldPosition )
		*worldPosition = FromB3( xform.p );
	if ( angles )
		*angles = FromB3( xform.q );
}

void CPhysicsObjectBox3D::GetPositionMatrix( matrix3x4_t *positionMatrix ) const
{
	b3WorldTransform xform = BodyTransform( m_body );
	Quaternion q( xform.q.v.x, xform.q.v.y, xform.q.v.z, xform.q.s );
	QuaternionMatrix( q, FromB3( xform.p ), *positionMatrix );
}

void CPhysicsObjectBox3D::LocalToWorld( Vector *worldPosition, const Vector &localPosition ) const
{
	*worldPosition = FromB3( b3TransformPoint( BodyTransform( m_body ), ToB3( localPosition ) ) );
}

void CPhysicsObjectBox3D::WorldToLocal( Vector *localPosition, const Vector &worldPosition ) const
{
	*localPosition = FromB3( b3InvTransformPoint( BodyTransform( m_body ), ToB3( worldPosition ) ) );
}

void CPhysicsObjectBox3D::LocalToWorldVector( Vector *worldVector, const Vector &localVector ) const
{
	*worldVector = FromB3( b3RotateVector( BodyTransform( m_body ).q, ToB3( localVector ) ) );
}

void CPhysicsObjectBox3D::WorldToLocalVector( Vector *localVector, const Vector &worldVector ) const
{
	*localVector = FromB3( b3InvRotateVector( BodyTransform( m_body ).q, ToB3( worldVector ) ) );
}

//-----------------------------------------------------------------------------
// Velocity and impulses
//-----------------------------------------------------------------------------
void CPhysicsObjectBox3D::SetWorldVelocity( const Vector &linear, const Vector &angularRadians )
{
	if ( b3Body_GetType( m_body ) == b3_staticBody )
		return;
	b3Body_SetLinearVelocity( m_body, ToB3( linear ) );
	b3Body_SetAngularVelocity( m_body, ToB3( angularRadians ) );
}

void CPhysicsObjectBox3D::GetWorldVelocity( Vector *linear, Vector *angularRadians ) const
{
	if ( linear )
		*linear = FromB3( b3Body_GetLinearVelocity( m_body ) );
	if ( angularRadians )
		*angularRadians = FromB3( b3Body_GetAngularVelocity( m_body ) );
}

void CPhysicsObjectBox3D::SetVelocity( const Vector *velocity, const AngularImpulse *angularVelocity )
{
	if ( !IsMoveable() )
		return;
	b3Quat rotation = BodyTransform( m_body ).q;
	if ( velocity )
		b3Body_SetLinearVelocity( m_body, ToB3( *velocity ) );
	if ( angularVelocity )
		b3Body_SetAngularVelocity( m_body, AngularToB3( *angularVelocity, rotation ) );
	Wake();
}

void CPhysicsObjectBox3D::SetVelocityInstantaneous( const Vector *velocity, const AngularImpulse *angularVelocity )
{
	SetVelocity( velocity, angularVelocity );
}

void CPhysicsObjectBox3D::CapturePreStepVelocity()
{
	GetWorldVelocity( &m_preStepLinear, &m_preStepAngular );
}

void CPhysicsObjectBox3D::GetVelocity( Vector *velocity, AngularImpulse *angularVelocity ) const
{
	if ( m_reportPreStep )
	{
		if ( velocity )
			*velocity = m_preStepLinear;
		if ( angularVelocity )
			*angularVelocity = AngularFromB3( ToB3( m_preStepAngular ), BodyTransform( m_body ).q );
		return;
	}
	if ( velocity )
		*velocity = FromB3( b3Body_GetLinearVelocity( m_body ) );
	if ( angularVelocity )
		*angularVelocity = AngularFromB3( b3Body_GetAngularVelocity( m_body ), BodyTransform( m_body ).q );
}

void CPhysicsObjectBox3D::AddVelocity( const Vector *velocity, const AngularImpulse *angularVelocity )
{
	if ( !IsMoveable() )
		return;
	Vector linear;
	AngularImpulse angular;
	GetVelocity( &linear, &angular );
	if ( velocity )
		linear += *velocity;
	if ( angularVelocity )
		angular += *angularVelocity;
	SetVelocity( &linear, &angular );
}

void CPhysicsObjectBox3D::GetVelocityAtPoint( const Vector &worldPosition, Vector *pVelocity ) const
{
	*pVelocity = FromB3( b3Body_GetWorldPointVelocity( m_body, ToB3( worldPosition ) ) );
}

void CPhysicsObjectBox3D::GetImplicitVelocity( Vector *velocity, AngularImpulse *angularVelocity ) const
{
	GetVelocity( velocity, angularVelocity );
}

void CPhysicsObjectBox3D::ApplyForceCenter( const Vector &forceVector )
{
	// VPhysics "forces" are impulses (kg * in/s).
	if ( IsMoveable() )
		b3Body_ApplyLinearImpulseToCenter( m_body, ToB3( forceVector ), true );
}

void CPhysicsObjectBox3D::ApplyForceOffset( const Vector &forceVector, const Vector &worldPosition )
{
	if ( IsMoveable() )
		b3Body_ApplyLinearImpulse( m_body, ToB3( forceVector ), ToB3( worldPosition ), true );
}

void CPhysicsObjectBox3D::ApplyTorqueCenter( const AngularImpulse &torque )
{
	if ( IsMoveable() )
		b3Body_ApplyAngularImpulse( m_body, AngularToB3( torque, BodyTransform( m_body ).q ), true );
}

void CPhysicsObjectBox3D::CalculateForceOffset( const Vector &forceVector, const Vector &worldPosition, Vector *centerForce, AngularImpulse *centerTorque ) const
{
	Vector center = FromB3( b3Body_GetWorldCenter( m_body ) );
	if ( centerForce )
		*centerForce = forceVector;
	if ( centerTorque )
	{
		Vector worldTorque = CrossProduct( worldPosition - center, forceVector );
		Vector local;
		WorldToLocalVector( &local, worldTorque );
		*centerTorque = AngularImpulse( RAD2DEG( local.x ), RAD2DEG( local.y ), RAD2DEG( local.z ) );
	}
}

void CPhysicsObjectBox3D::CalculateVelocityOffset( const Vector &forceVector, const Vector &worldPosition, Vector *centerVelocity, AngularImpulse *centerAngularVelocity ) const
{
	Vector force;
	AngularImpulse torque;
	CalculateForceOffset( forceVector, worldPosition, &force, &torque );
	if ( centerVelocity )
		*centerVelocity = force * GetInvMass();
	if ( centerAngularVelocity )
	{
		Vector invInertia = GetInvInertia();
		*centerAngularVelocity = AngularImpulse( torque.x * invInertia.x, torque.y * invInertia.y, torque.z * invInertia.z );
	}
}

bool CPhysicsObjectBox3D::GetContactPoint( Vector *contactPoint, IPhysicsObject **contactObject ) const
{
	CFrictionSnapshotBox3D snapshot( const_cast<CPhysicsObjectBox3D *>( this ) );
	if ( !snapshot.IsValid() )
		return false;
	if ( contactPoint )
		snapshot.GetContactPoint( *contactPoint );
	if ( contactObject )
		*contactObject = snapshot.GetObject( 1 );
	return true;
}

//-----------------------------------------------------------------------------
// Shadows
//-----------------------------------------------------------------------------
IPhysicsShadowController *CPhysicsObjectBox3D::EnsureShadowController( bool allowTranslation, bool allowRotation )
{
	if ( !m_pShadow )
	{
		m_pShadow = new CShadowControllerBox3D( this, allowTranslation, allowRotation );
		ApplyMassProperties();
		RecheckCollisionFilter();
	}
	return m_pShadow;
}

void CPhysicsObjectBox3D::SetShadow( float maxSpeed, float maxAngularSpeed, bool allowPhysicsMovement, bool allowPhysicsRotation )
{
	EnsureShadowController( allowPhysicsMovement, allowPhysicsRotation )->MaxSpeed( maxSpeed, maxAngularSpeed );
}

void CPhysicsObjectBox3D::UpdateShadow( const Vector &targetPosition, const QAngle &targetAngles, bool tempDisableGravity, float timeOffset )
{
	if ( m_pShadow )
	{
		m_pShadow->SetTempDisableGravity( tempDisableGravity );
		m_pShadow->Update( targetPosition, targetAngles, timeOffset );
	}
}

int CPhysicsObjectBox3D::GetShadowPosition( Vector *position, QAngle *angles ) const
{
	GetPosition( position, angles );
	return 1;
}

IPhysicsShadowController *CPhysicsObjectBox3D::GetShadowController( void ) const
{
	return m_pShadow;
}

void CPhysicsObjectBox3D::RemoveShadowController()
{
	delete m_pShadow;
	m_pShadow = NULL;
	ApplyMassProperties();
}

float CPhysicsObjectBox3D::ComputeShadowControl( const hlshadowcontrol_params_t &params, float secondsToArrival, float dt )
{
	// Called by game motion events (e.g. the grab controller carrying a held
	// object) from inside the step: steer this object's velocity directly.
	return ComputeShadowControlBox3D( this, params, secondsToArrival, dt, NULL );
}

//-----------------------------------------------------------------------------
// Triggers, hinges, contacts
//-----------------------------------------------------------------------------
void CPhysicsObjectBox3D::BecomeTrigger()
{
	m_isTrigger = true;
	ApplyFilter();
}

void CPhysicsObjectBox3D::RemoveTrigger()
{
	m_isTrigger = false;
	ApplyFilter();
}

void CPhysicsObjectBox3D::BecomeHinged( int localAxis )
{
	if ( m_isStatic || localAxis < 0 || localAxis > 2 )
		return;
	m_hingeAxis = localAxis;
	// Rotation is limited to the hinge axis; translation stays free, as in IVP.
	b3MotionLocks locks;
	memset( &locks, 0, sizeof( locks ) );
	locks.angularX = localAxis != 0;
	locks.angularY = localAxis != 1;
	locks.angularZ = localAxis != 2;
	b3Body_SetMotionLocks( m_body, locks );
}

void CPhysicsObjectBox3D::RemoveHinged()
{
	m_hingeAxis = -1;
	b3MotionLocks locks;
	memset( &locks, 0, sizeof( locks ) );
	if ( !m_isStatic )
		b3Body_SetMotionLocks( m_body, locks );
}

IPhysicsFrictionSnapshot *CPhysicsObjectBox3D::CreateFrictionSnapshot()
{
	return new CFrictionSnapshotBox3D( this );
}

void CPhysicsObjectBox3D::DestroyFrictionSnapshot( IPhysicsFrictionSnapshot *pSnapshot )
{
	delete pSnapshot;
}

// Same report as IVP's CPhysicsObject::OutputDebugInfo (physics_debug_entity),
// plus the pose, which Box3D can report directly.
void CPhysicsObjectBox3D::OutputDebugInfo() const
{
	Msg( "-----------------\nObject: %s\n", m_name );
	Msg( "Mass: %.3e (inv %.3e)\n", GetMass(), GetInvMass() );
	Vector invInertia = GetInvInertia();
	Msg( "Inertia: %.3e, %.3e, %.3e (inv %.3e, %.3e, %.3e)\n", m_inertia.x, m_inertia.y, m_inertia.z, invInertia.x, invInertia.y, invInertia.z );
	Vector position;
	QAngle angles;
	GetPosition( &position, &angles );
	Msg( "Position: %.2f, %.2f, %.2f Angles: %.2f, %.2f, %.2f\n", position.x, position.y, position.z, angles.x, angles.y, angles.z );
	Vector speed;
	AngularImpulse angSpeed;
	GetVelocity( &speed, &angSpeed );
	Msg( "Velocity: %.2f, %.2f, %.2f \n", speed.x, speed.y, speed.z );
	Msg( "Ang Velocity: %.2f, %.2f, %.2f \n", angSpeed.x, angSpeed.y, angSpeed.z );
	Msg( "Damping %.3e linear, %.3e angular\n", m_linearDamping, m_angularDamping );
	if ( IsHinged() )
	{
		const char *pAxisNames[] = { "x", "y", "z" };
		Msg( "Hinged on %s axis\n", pAxisNames[m_hingeAxis] );
	}
	Msg( "Shadow controller: %s, player controller: %s\n", m_pShadow ? "yes" : "no", m_pPlayerController ? "yes" : "no" );
	static const char *s_bodyTypes[] = { "static", "kinematic", "dynamic" };
	b3BodyType type = b3Body_GetType( m_body );
	Vector gravity;
	m_pEnv->GetGravity( &gravity );
	Msg( "Box3D body: %s, gravity scale %.2f; environment gravity %.1f %.1f %.1f, %d steps over %.2f s\n",
		type >= 0 && type <= 2 ? s_bodyTypes[type] : "?", b3Body_GetGravityScale( m_body ),
		gravity.x, gravity.y, gravity.z, m_pEnv->GetStepCount(), m_pEnv->GetSimulationTime() );
	Msg( "State: %s, Collision %s, Motion %s, Flags %04X (game %04x, index %d)\n",
		IsAsleep() ? "Asleep" : "Awake",
		IsCollisionEnabled() ? "Enabled" : "Disabled",
		IsStatic() ? "Static" : ( IsMotionEnabled() ? "Enabled" : "Disabled" ),
		(int)GetCallbackFlags(), (int)GetGameFlags(), (int)GetGameIndex() );
	float density = 0, thickness = 0, friction = 0, elasticity = 0;
	g_SurfaceDatabase.GetPhysicsProperties( m_materialIndex, &density, &thickness, &friction, &elasticity );
	Msg( "Material: %s : density(%.1f), thickness(%.2f), friction(%.2f), elasticity(%.2f)\n",
		g_SurfaceDatabase.GetPropName( m_materialIndex ), density, thickness, friction, elasticity );
}
