//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics object backed by a Box3D body (RFC 0004 B2/B5/C6).
//
//=============================================================================//
#include "physics_object.h"

#include <math.h>
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
// A body's shapes (one per hull; triangle meshes have one per triangle).
void GetBodyShapes( b3BodyId body, CUtlVector<b3ShapeId> &shapes )
{
	shapes.SetCount( b3Body_GetShapeCount( body ) );
	if ( shapes.Count() )
		shapes.SetCount( b3Body_GetShapes( body, shapes.Base(), shapes.Count() ) );
}
const float kMetersPerInch = 0.0254f;

b3WorldTransform BodyTransform( b3BodyId body )
{
	return b3Body_GetTransform( body );
}

// IVP's AngDragIntegral (vphysics/physics_object.cpp): the integral of each
// differential drag area's torque over one pair of opposite OBB faces.
float AngDragIntegral( float invInertia, float l, float w, float h )
{
	float w2 = w * w;
	float l2 = l * l;
	float h2 = h * h;
	return invInertia * ( ( 1.f / 3.f ) * w2 * l * l2 + 0.5f * w2 * w2 * l + l * w2 * h2 );
}
}

CPhysicsObjectBox3D::CPhysicsObjectBox3D( CPhysicsEnvironmentBox3D *pEnv, const CPhysCollide *pCollide, float sphereRadius,
	int materialIndex, const Vector &position, const QAngle &angles, const objectparams_t *pParams, bool isStatic )
	: m_pEnv( pEnv ), m_pCollide( pCollide ), m_body( b3_nullBodyId ), m_pShadow( NULL ), m_pPlayerController( NULL ),
	  m_pFluid( NULL ), m_pGameData( pParams ? pParams->pGameData : NULL ), m_sphereRadius( sphereRadius ),
	  m_volume( 0.0f ), m_buoyancyRatio( 1.0f ), m_friction( 0.8f ), m_restitution( 0.0f ), m_hingeAxis( -1 ),
	  m_contents( CONTENTS_SOLID ), m_callbackFlags( kDefaultCallbacks ), m_gameFlags( 0 ), m_gameIndex( 0 ),
	  m_isStatic( isStatic ), m_isTrigger( false ), m_collisionEnabled( pParams ? pParams->enableCollisions : true ),
	  m_gravityEnabled( !isStatic ), m_dragEnabled( false ), m_motionEnabled( true ), m_shadowTempGravityDisable( false ),
	  m_wasAwake( false ), m_reportPreStep( false ), m_hasTouchedDynamic( false ), m_asleepSinceCreation( true )
{
	objectparams_t defaults;
	memset( &defaults, 0, sizeof( defaults ) );
	defaults.mass = 1.0f;
	defaults.inertia = 1.0f;
	defaults.enableCollisions = true;
	const objectparams_t &params = pParams ? *pParams : defaults;

	m_preStepLinear.Init();
	m_preStepAngular.Init();
	V_strncpy( m_name, params.pName ? params.pName : "box3d_object", sizeof( m_name ) );

	// IVP's InitObjectTemplate.
	m_mass = clamp( params.mass, VPHYSICS_MIN_MASS, VPHYSICS_MAX_MASS );
	m_inertiaScale = params.inertia <= 0 ? 1.0f : MIN( params.inertia, 1e14f );
	m_rotInertiaLimit = params.rotInertiaLimit;
	m_speedDamping = params.damping;
	m_rotDamping = params.rotdamping;
	if ( materialIndex < 0 )
		materialIndex = g_SurfaceDatabase.GetSurfaceIndex( "default" );
	m_materialIndex = materialIndex;

	const CPhysCollideBox3D *pBox = pCollide ? ToBox3D( pCollide ) : NULL;
	m_massCenter = pBox ? pBox->massCenter : vec3_origin;
	if ( params.massCenterOverride )
		m_massCenter = *params.massCenterOverride;

	surfacedata_t *pSurface = g_SurfaceDatabase.GetSurfaceData( materialIndex );
	if ( pSurface )
	{
		m_friction = pSurface->physics.friction;
		m_restitution = pSurface->physics.elasticity;
	}

	// Drag applies to non-static polygon objects with a drag coefficient;
	// spheres never get drag (IVP's CreatePhysicsSphere).
	bool dragCapable = !isStatic && pCollide != NULL;
	m_dragCoefficient = dragCapable ? params.dragCoefficient : 0.0f;
	m_angDragCoefficient = m_dragCoefficient;
	m_dragBasis.Init();
	m_angDragBasis.Init();

	CreateBody( position, angles );
	ComputeInitialInertia();

	// IVP's SetVolume: buoyancy compares the object's density (mass over
	// collision volume) with its material's.
	float volume = params.volume;
	if ( m_sphereRadius > 0.0f && volume <= 0.0f )
		volume = 4.0f * M_PI_F * m_sphereRadius * m_sphereRadius * m_sphereRadius / 3.0f;
	m_volume = volume;
	UpdateBuoyancyRatio();

	RecomputeDragBases();
	ApplyMassProperties();
	m_dragEnabled = dragCapable && params.dragCoefficient != 0.0f;
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

//-----------------------------------------------------------------------------
// State capture: transfer, serialization, save/restore
//-----------------------------------------------------------------------------
void CPhysicsObjectBox3D::CreateBody( const Vector &position, const QAngle &angles )
{
	b3BodyDef def = b3DefaultBodyDef();
	def.type = m_isStatic ? b3_staticBody : b3_dynamicBody;
	def.position = ToB3( position );
	def.rotation = ToB3( angles );
	// IVP damping is applied by the environment each step.
	def.linearDamping = 0.0f;
	def.angularDamping = 0.0f;
	def.userData = this;
	def.name = m_name;
	def.isAwake = false;
	m_body = b3CreateBody( m_pEnv->GetWorld(), &def );
	CreateShapes();
}

void CPhysicsObjectBox3D::WriteState( CPhysicsObjectStateBox3D &state ) const
{
	memset( &state, 0, sizeof( state ) );
	state.version = kPhysicsObjectStateVersion;
	state.pCollide = m_pCollide;
	state.sphereRadius = m_sphereRadius;
	state.isStatic = m_isStatic;
	state.collisionEnabled = m_collisionEnabled;
	state.gravityEnabled = m_gravityEnabled;
	state.dragEnabled = m_dragEnabled;
	state.motionEnabled = m_motionEnabled;
	state.isAsleep = IsAsleep();
	state.isTrigger = m_isTrigger;
	state.asleepSinceCreation = m_asleepSinceCreation;
	state.hasTouchedDynamic = m_hasTouchedDynamic;
	state.materialIndex = m_materialIndex;
	state.mass = m_mass;
	state.inertia = m_inertia;
	state.inertiaScale = m_inertiaScale;
	state.rotInertiaLimit = m_rotInertiaLimit;
	state.speedDamping = m_speedDamping;
	state.rotDamping = m_rotDamping;
	state.massCenter = m_massCenter;
	state.callbacks = m_callbackFlags;
	state.gameFlags = m_gameFlags;
	state.gameIndex = m_gameIndex;
	state.contents = m_contents;
	state.volume = m_volume;
	state.dragCoefficient = m_dragCoefficient;
	state.angDragCoefficient = m_angDragCoefficient;
	state.hingeAxis = m_hingeAxis;
	GetPosition( &state.origin, &state.angles );
	GetWorldVelocity( &state.velocity, &state.angularVelocity );
	V_strncpy( state.name, m_name, sizeof( state.name ) );
}

// IVP's CPhysicsObject::InitFromTemplate order: authored properties, then
// motion/trigger/gravity/collision state, then velocity or sleep.
void CPhysicsObjectBox3D::ApplyState( const CPhysicsObjectStateBox3D &state, bool enableCollisions )
{
	m_mass = state.mass;
	m_inertia = state.inertia;
	m_inertiaScale = state.inertiaScale;
	m_rotInertiaLimit = state.rotInertiaLimit;
	m_speedDamping = state.speedDamping;
	m_rotDamping = state.rotDamping;
	m_massCenter = state.massCenter;
	m_callbackFlags = (unsigned short)state.callbacks;
	m_gameFlags = (unsigned short)state.gameFlags;
	m_gameIndex = (unsigned short)state.gameIndex;
	m_contents = state.contents;
	m_volume = state.volume;
	m_dragCoefficient = state.dragCoefficient;
	m_angDragCoefficient = state.angDragCoefficient;
	m_asleepSinceCreation = state.asleepSinceCreation;
	m_hasTouchedDynamic = state.hasTouchedDynamic;
	UpdateBuoyancyRatio();
	RecomputeDragBases();
	ApplyMassProperties();

	m_dragEnabled = !m_isStatic && state.dragEnabled;
	if ( !m_isStatic && !state.motionEnabled )
	{
		m_motionEnabled = false;
		ApplyBodyType();
	}
	if ( state.isTrigger && !m_isTrigger )
		BecomeTrigger();
	m_gravityEnabled = !m_isStatic && state.gravityEnabled;
	m_collisionEnabled = state.collisionEnabled && enableCollisions;
	ApplyFilter();

	if ( state.velocity.LengthSqr() != 0 || state.angularVelocity.LengthSqr() != 0 )
	{
		Wake();
		SetWorldVelocity( state.velocity, state.angularVelocity );
	}
	else if ( !state.isAsleep && !m_isStatic )
	{
		Wake();
	}
	if ( state.isAsleep )
		Sleep();
	if ( state.hingeAxis >= 0 )
		BecomeHinged( state.hingeAxis );
	m_wasAwake = !IsAsleep();
}

CPhysicsObjectBox3D *CPhysicsObjectBox3D::CreateFromState( CPhysicsEnvironmentBox3D *pEnv, void *pGameData,
	const CPhysicsObjectStateBox3D &state, bool enableCollisions )
{
	if ( state.version != kPhysicsObjectStateVersion )
		return NULL;
	objectparams_t params;
	memset( &params, 0, sizeof( params ) );
	params.mass = state.mass;
	params.inertia = state.inertiaScale;
	params.rotInertiaLimit = state.rotInertiaLimit;
	params.damping = state.speedDamping;
	params.rotdamping = state.rotDamping;
	params.pName = state.name;
	params.pGameData = pGameData;
	params.volume = state.volume;
	params.dragCoefficient = state.dragCoefficient;
	params.enableCollisions = false;
	Vector massCenter = state.massCenter;
	params.massCenterOverride = &massCenter;
	CPhysicsObjectBox3D *pObject = new CPhysicsObjectBox3D( pEnv, state.pCollide, state.sphereRadius, state.materialIndex,
		state.origin, state.angles, &params, state.isStatic );
	pObject->ApplyState( state, enableCollisions );
	return pObject;
}

void CPhysicsObjectBox3D::MoveToEnvironment( CPhysicsEnvironmentBox3D *pDestination )
{
	CPhysicsObjectStateBox3D state;
	WriteState( state );
	bool trigger = m_isTrigger;
	if ( b3Body_IsValid( m_body ) )
		b3DestroyBody( m_body );
	m_pEnv = pDestination;
	// Rebuild with the same shape kind, then restore the state; the trigger
	// is already reflected in the shapes.
	m_isTrigger = trigger;
	m_motionEnabled = true;
	CreateBody( state.origin, state.angles );
	ApplyState( state, true );
}

//-----------------------------------------------------------------------------
// Shapes and mass properties
//-----------------------------------------------------------------------------
void CPhysicsObjectBox3D::CreateShapes()
{
	b3ShapeDef def = b3DefaultShapeDef();
	def.userData = this;
	def.enableCustomFiltering = true;
	def.enableContactEvents = true;
	// Contacts carry the pre-solve flag from creation, so every shape has it:
	// the environment holds off contacts the game deleted (friction snapshot
	// DeleteAllMarkedContacts). The callback returns at once when none are.
	def.enablePreSolveEvents = true;
	def.enableHitEvents = true;
	// Every shape is visible to triggers and fluids (Box3D sensors).
	def.enableSensorEvents = true;
	def.isSensor = m_isTrigger;
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

void CPhysicsObjectBox3D::DestroyShapes()
{
	CUtlVector<b3ShapeId> shapes;
	GetBodyShapes( m_body, shapes );
	int shapeCount = shapes.Count();
	for ( int i = 0; i < shapeCount; i++ )
		b3DestroyShape( shapes[i], false );
}

int CPhysicsObjectBox3D::GetShapes( b3ShapeId *pShapes, int capacity ) const
{
	return b3Body_GetShapes( m_body, pShapes, capacity );
}

// IVP_Real_Object's template: inertia = mass * per-mass inertia * scale,
// clipped below at |I| * rotInertiaLimit.
void CPhysicsObjectBox3D::ComputeInitialInertia()
{
	Vector perMass( 1, 1, 1 );
	if ( m_sphereRadius > 0.0f )
	{
		float r = m_sphereRadius * kMetersPerInch;
		perMass.Init( 0.4f * r * r, 0.4f * r * r, 0.4f * r * r );
	}
	else if ( m_pCollide )
	{
		perMass = ToBox3D( m_pCollide )->rotationInertia;
	}
	m_inertia = perMass * ( m_mass * m_inertiaScale );
	if ( m_rotInertiaLimit != 0.0f )
	{
		float minimum = m_inertia.Length() * m_rotInertiaLimit;
		for ( int i = 0; i < 3; i++ )
			m_inertia[i] = MAX( m_inertia[i], minimum );
	}
}

void CPhysicsObjectBox3D::ApplyMassProperties()
{
	if ( m_isStatic || b3Body_GetType( m_body ) != b3_dynamicBody )
		return;
	b3MassData mass;
	mass.mass = m_mass;
	mass.center = ToB3( m_massCenter );
	// IVP expresses "cannot rotate" as ~1e14 kg*m^2 (shadows, then scaled by
	// their raised mass). Box3D inverts the tensor through its determinant,
	// which overflows float for such values and turns the inverse into NaN,
	// so the simulated inertia is capped at a value that is still effectively
	// infinite; GetInertia keeps reporting IVP's value.
	const float kMaxBox3DInertia = 1e12f;
	Vector inertia = m_inertia * kInertiaToBox3D;
	for ( int i = 0; i < 3; i++ )
		inertia[i] = clamp( inertia[i], 1e-6f, kMaxBox3DInertia );
	mass.inertia.cx = { inertia.x, 0, 0 };
	mass.inertia.cy = { 0, inertia.y, 0 };
	mass.inertia.cz = { 0, 0, inertia.z };
	b3Body_SetMassData( m_body, mass );
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
	// Refiltering re-evaluates contacts, which wakes the body in Box3D; a
	// filter change alone does not wake an IVP object.
	bool asleep = !m_isStatic && IsAsleep();
	CUtlVector<b3ShapeId> shapes;
	GetBodyShapes( m_body, shapes );
	int shapeCount = shapes.Count();
	for ( int i = 0; i < shapeCount; i++ )
	{
		b3Filter filter = b3Shape_GetFilter( shapes[i] );
		filter.categoryBits = m_collisionEnabled ? B3_DEFAULT_CATEGORY_BITS : 0;
		filter.maskBits = m_collisionEnabled ? B3_DEFAULT_MASK_BITS : 0;
		b3Shape_SetFilter( shapes[i], filter, true );
	}
	if ( !m_isStatic )
		b3Body_SetGravityScale( m_body, m_gravityEnabled ? 1.0f : 0.0f );
	if ( asleep && !IsAsleep() )
		b3Body_SetAwake( m_body, false );
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
	if ( m_isStatic )
		return;
	m_gravityEnabled = enable;
	b3Body_SetGravityScale( m_body, enable ? 1.0f : 0.0f );
}

void CPhysicsObjectBox3D::EnableDrag( bool enable )
{
	if ( m_isStatic )
		return;
	m_dragEnabled = enable;
}

void CPhysicsObjectBox3D::SetDragCoefficient( float *pDrag, float *pAngularDrag )
{
	if ( pDrag )
		m_dragCoefficient = *pDrag;
	if ( pAngularDrag )
		m_angDragCoefficient = *pAngularDrag;
	EnableDrag( m_dragCoefficient != 0 || m_angDragCoefficient != 0 );
}

void CPhysicsObjectBox3D::EnableMotion( bool enable )
{
	if ( m_isStatic || m_motionEnabled == enable )
		return;
	m_motionEnabled = enable;
	ApplyBodyType();
	if ( enable && IsHinged() )
		BecomeHinged( m_hingeAxis );
	RecheckCollisionFilter();
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
	if ( IsMarkedForDelete() )
		return;
	ApplyFilter();
	// Unchanged filter bits leave Box3D's contacts as they are; the game's
	// collision rules (portal environments) are re-run on the object's pairs.
	if ( m_pEnv )
		m_pEnv->RecheckPairs( this );
}

void CPhysicsObjectBox3D::RecheckContactPoints( void )
{
	RecheckCollisionFilter();
}

void CPhysicsObjectBox3D::UpdateBuoyancyRatio()
{
	// IVP's SetVolume (minimum 5 cubic inches for stability).
	if ( m_volume != 0.0f )
	{
		float volume = MAX( m_volume, 5.0f ) * kMetersPerInch * kMetersPerInch * kMetersPerInch;
		float density = m_mass / volume;
		float matDensity = 1.0f;
		g_SurfaceDatabase.GetPhysicsProperties( m_materialIndex, &matDensity, NULL, NULL, NULL );
		m_buoyancyRatio = matDensity > 0.0f ? density / matDensity : 1.0f;
	}
	else
	{
		m_buoyancyRatio = 1.0f;
	}
}

void CPhysicsObjectBox3D::SetMass( float mass )
{
	// IVP clamps runtime mass changes to [1, max] and rescales the inertia
	// (IVP_Core::set_mass).
	mass = clamp( mass, 1.0f, VPHYSICS_MAX_MASS );
	if ( m_mass > 0.0f )
		m_inertia *= mass / m_mass;
	m_mass = mass;
	UpdateBuoyancyRatio();
	RecomputeDragBases();
	ApplyMassProperties();
}

float CPhysicsObjectBox3D::GetInvMass( void ) const
{
	// IVP reports the core's inverse mass: pinned cores have none, and a
	// static core keeps its authored mass.
	if ( !m_isStatic && !m_motionEnabled )
		return 0.0f;
	return m_mass > 0.0f ? 1.0f / m_mass : 0.0f;
}

Vector CPhysicsObjectBox3D::GetInvInertia( void ) const
{
	if ( !m_isStatic && !m_motionEnabled )
		return vec3_origin;
	return Vector( m_inertia.x > 0 ? 1.0f / m_inertia.x : 0.0f, m_inertia.y > 0 ? 1.0f / m_inertia.y : 0.0f,
		m_inertia.z > 0 ? 1.0f / m_inertia.z : 0.0f );
}

void CPhysicsObjectBox3D::SetInertia( const Vector &inertia )
{
	m_inertia.Init( fabsf( inertia.x ), fabsf( inertia.y ), fabsf( inertia.z ) );
	ApplyMassProperties();
}

void CPhysicsObjectBox3D::SetDamping( const float *speed, const float *rot )
{
	if ( speed )
		m_speedDamping = *speed;
	if ( rot )
		m_rotDamping = *rot;
}

void CPhysicsObjectBox3D::GetDamping( float *speed, float *rot ) const
{
	if ( speed )
		*speed = m_speedDamping;
	if ( rot )
		*rot = m_rotDamping;
}

void CPhysicsObjectBox3D::SetMaterialIndex( int materialIndex )
{
	if ( m_materialIndex == materialIndex )
		return;
	m_materialIndex = materialIndex;
	surfacedata_t *pSurface = g_SurfaceDatabase.GetSurfaceData( materialIndex );
	if ( pSurface )
	{
		m_friction = pSurface->physics.friction;
		m_restitution = pSurface->physics.elasticity;
	}
	CUtlVector<b3ShapeId> shapes;
	GetBodyShapes( m_body, shapes );
	int shapeCount = shapes.Count();
	for ( int i = 0; i < shapeCount; i++ )
	{
		b3SurfaceMaterial material = b3Shape_GetSurfaceMaterial( shapes[i] );
		material.friction = m_friction;
		material.restitution = m_restitution;
		material.userMaterialId = (uint64_t)m_materialIndex;
		b3Shape_SetSurfaceMaterial( shapes[i], material );
	}
	if ( m_pShadow )
		m_pShadow->ObjectMaterialChanged( materialIndex );
}

// IVP: 1/2 m v^2 + 1/2 w.I.w in Havok units, converted to Source energy.
float CPhysicsObjectBox3D::GetEnergy() const
{
	if ( m_isStatic )
		return 0.0f;
	Vector linear, angular;
	GetWorldVelocity( &linear, &angular );
	Vector localAngular;
	WorldToLocalVector( &localAngular, angular );
	float rotational = m_inertia.x * localAngular.x * localAngular.x + m_inertia.y * localAngular.y * localAngular.y +
		m_inertia.z * localAngular.z * localAngular.z;
	return 0.5f * ( m_mass * linear.LengthSqr() + rotational * kInertiaToBox3D );
}

//-----------------------------------------------------------------------------
// Drag (IVP's CPhysicsObject::RecomputeDragBases / CDragController). The
// bases are computed in IVP's axis order (x, -z, y) exactly as IVP does,
// including its pairing of the y/z area fractions, then stored per Source
// axis.
//-----------------------------------------------------------------------------
void CPhysicsObjectBox3D::RecomputeDragBases()
{
	if ( m_isStatic || !m_pCollide )
		return;
	const CPhysCollideBox3D *pBox = ToBox3D( m_pCollide );
	Vector areaFractions = pBox->orthoAreas;
	Vector delta = pBox->maxs - pBox->mins;
	float dX = fabsf( delta.x ) * kMetersPerInch;
	float dY = fabsf( delta.z ) * kMetersPerInch;	// IVP y is Source -z
	float dZ = fabsf( delta.y ) * kMetersPerInch;	// IVP z is Source y
	float invMass = m_mass > 0.0f ? 1.0f / m_mass : 0.0f;
	float basisIvp[3] = { dY * dZ * areaFractions.x * invMass, dX * dZ * areaFractions.y * invMass,
		dX * dY * areaFractions.z * invMass };
	m_dragBasis.Init( basisIvp[0], basisIvp[2], basisIvp[1] );

	float invInertiaIvp[3] = { m_inertia.x > 0 ? 1.0f / m_inertia.x : 0.0f, m_inertia.z > 0 ? 1.0f / m_inertia.z : 0.0f,
		m_inertia.y > 0 ? 1.0f / m_inertia.y : 0.0f };
	float hX = 0.5f * dX, hY = 0.5f * dY, hZ = 0.5f * dZ;
	float angIvp[3];
	angIvp[0] = areaFractions.z * AngDragIntegral( invInertiaIvp[0], hX, hY, hZ ) + areaFractions.y * AngDragIntegral( invInertiaIvp[0], hX, hZ, hY );
	angIvp[1] = areaFractions.z * AngDragIntegral( invInertiaIvp[1], hY, hX, hZ ) + areaFractions.x * AngDragIntegral( invInertiaIvp[1], hY, hZ, hX );
	angIvp[2] = areaFractions.y * AngDragIntegral( invInertiaIvp[2], hZ, hX, hY ) + areaFractions.x * AngDragIntegral( invInertiaIvp[2], hZ, hY, hX );
	m_angDragBasis.Init( angIvp[0], angIvp[2], angIvp[1] );
}

// IVP's GetDragInDirection, velocity in Havok units (m/s), world space. Note
// IVP applies the coefficient to the x term only (operator precedence in
// the original); content is tuned against that, so it is kept.
float CPhysicsObjectBox3D::GetDragInDirection( const Vector &worldVelocity ) const
{
	Vector local;
	WorldToLocalVector( &local, worldVelocity );
	return m_dragCoefficient * fabsf( local.x * m_dragBasis.x ) + fabsf( local.y * m_dragBasis.y ) + fabsf( local.z * m_dragBasis.z );
}

float CPhysicsObjectBox3D::GetAngularDragInDirection( const Vector &localAngularRadians ) const
{
	return m_angDragCoefficient * fabsf( localAngularRadians.x * m_angDragBasis.x ) + fabsf( localAngularRadians.y * m_angDragBasis.y ) +
		fabsf( localAngularRadians.z * m_angDragBasis.z );
}

float CPhysicsObjectBox3D::CalculateLinearDrag( const Vector &unitDirection ) const
{
	return GetDragInDirection( unitDirection );
}

float CPhysicsObjectBox3D::CalculateAngularDrag( const Vector &objectSpaceRotationAxis ) const
{
	// Drag factor is per radian; convert to per degree.
	return GetAngularDragInDirection( objectSpaceRotationAxis ) * DEG2RAD( 1.0f );
}

void CPhysicsObjectBox3D::ApplyDampingAndDrag( float dt, float airDensity )
{
	if ( !IsMoveable() || IsAsleep() || b3Body_GetType( m_body ) != b3_dynamicBody )
		return;
	Vector linear, angular;
	GetWorldVelocity( &linear, &angular );

	if ( IsDragEnabled() )
	{
		float dragForce = -0.5f * GetDragInDirection( linear * kMetersPerInch ) * airDensity * dt;
		if ( dragForce < -1.0f )
			dragForce = -1.0f;
		if ( dragForce < 0 )
			linear += linear * dragForce;
		Vector localAngular;
		WorldToLocalVector( &localAngular, angular );
		float angDragForce = -GetAngularDragInDirection( localAngular ) * airDensity * dt;
		if ( angDragForce < -1.0f )
			angDragForce = -1.0f;
		if ( angDragForce < 0 )
			angular += angular * angDragForce;
	}

	// IVP_Core::damp_object: the rotational factor switches to exp() once
	// the damping vector's squared length reaches 0.5, the linear one once
	// the scaled factor reaches 0.25.
	float rot = m_rotDamping * dt;
	float rotFactor = 3.0f * rot * rot < 0.5f ? 1.0f - rot : expf( -rot );
	float speed = m_speedDamping * dt;
	float speedFactor = speed < 0.25f ? 1.0f - speed : expf( -speed );
	linear *= speedFactor;
	angular *= rotFactor;
	SetWorldVelocity( linear, angular );
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
	// As IVP: moving a shadow-controlled object also retargets its shadow.
	if ( m_pShadow )
		UpdateShadow( worldPosition, angles, false, 0 );
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

// IVP's velocity limit (anomaly limits), not applied to shadow objects.
void CPhysicsObjectBox3D::ClampVelocity()
{
	if ( m_pShadow )
		return;
	physics_performanceparams_t performance;
	m_pEnv->GetPerformanceSettings( &performance );
	Vector linear, angular;
	GetWorldVelocity( &linear, &angular );
	bool changed = false;
	float speed = linear.Length();
	if ( performance.maxVelocity > 0 && speed > performance.maxVelocity )
	{
		linear *= performance.maxVelocity / speed;
		changed = true;
	}
	float angularSpeed = RAD2DEG( angular.Length() );
	if ( performance.maxAngularVelocity > 0 && angularSpeed > performance.maxAngularVelocity )
	{
		angular *= performance.maxAngularVelocity / angularSpeed;
		changed = true;
	}
	if ( changed )
		SetWorldVelocity( linear, angular );
}

void CPhysicsObjectBox3D::SetVelocity( const Vector *velocity, const AngularImpulse *angularVelocity )
{
	if ( !IsMoveable() )
		return;
	Wake();
	b3Quat rotation = BodyTransform( m_body ).q;
	if ( velocity )
		b3Body_SetLinearVelocity( m_body, ToB3( *velocity ) );
	if ( angularVelocity )
		b3Body_SetAngularVelocity( m_body, AngularToB3( *angularVelocity, rotation ) );
	ClampVelocity();
}

bool CPhysicsObjectBox3D::IsControlledByGame() const
{
	if ( m_pShadow && !m_pShadow->IsPhysicallyControlled() )
		return true;
	return ( m_callbackFlags & CALLBACK_IS_PLAYER_CONTROLLER ) != 0;
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
	Wake();
	Vector linear;
	AngularImpulse angular;
	GetVelocity( &linear, &angular );
	if ( velocity )
		linear += *velocity;
	if ( angularVelocity )
		angular += *angularVelocity;
	b3Quat rotation = BodyTransform( m_body ).q;
	b3Body_SetLinearVelocity( m_body, ToB3( linear ) );
	b3Body_SetAngularVelocity( m_body, AngularToB3( angular, rotation ) );
	ClampVelocity();
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
	if ( !IsMoveable() )
		return;
	b3Body_ApplyLinearImpulseToCenter( m_body, ToB3( forceVector ), true );
	ClampVelocity();
}

void CPhysicsObjectBox3D::ApplyForceOffset( const Vector &forceVector, const Vector &worldPosition )
{
	if ( !IsMoveable() )
		return;
	b3Body_ApplyLinearImpulse( m_body, ToB3( forceVector ), ToB3( worldPosition ), true );
	ClampVelocity();
}

void CPhysicsObjectBox3D::ApplyTorqueCenter( const AngularImpulse &torque )
{
	// IVP takes a WORLD-space angular impulse in kg*m^2*degrees/s
	// (IVP_Core::async_rot_push_core_multiple_ws).
	if ( !IsMoveable() )
		return;
	Vector radians( DEG2RAD( torque.x ), DEG2RAD( torque.y ), DEG2RAD( torque.z ) );
	b3Body_ApplyAngularImpulse( m_body, ToB3( radians * kInertiaToBox3D ), true );
	ClampVelocity();
}

// IVP: the torque about the mass center, returned in OBJECT space in Havok
// units (kg*m^2*degrees/s), and the resulting local angular velocity.
void CPhysicsObjectBox3D::CalculateForceOffset( const Vector &forceVector, const Vector &worldPosition, Vector *centerForce, AngularImpulse *centerTorque ) const
{
	Vector center = FromB3( b3Body_GetWorldCenter( m_body ) );
	if ( centerForce )
		*centerForce = forceVector;
	if ( centerTorque )
	{
		Vector worldTorque = CrossProduct( worldPosition - center, forceVector ) * ( kMetersPerInch * kMetersPerInch );
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
	float invMass = m_mass > 0.0f ? 1.0f / m_mass : 0.0f;
	if ( centerVelocity )
		*centerVelocity = force * invMass;
	if ( centerAngularVelocity )
	{
		Vector invInertia( m_inertia.x > 0 ? 1.0f / m_inertia.x : 0.0f, m_inertia.y > 0 ? 1.0f / m_inertia.y : 0.0f,
			m_inertia.z > 0 ? 1.0f / m_inertia.z : 0.0f );
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
		m_shadowTempGravityDisable = false;
		m_pShadow = new CShadowControllerBox3D( this, allowTranslation, allowRotation );
		RecheckCollisionFilter();
	}
	return m_pShadow;
}

CShadowControllerBox3D *CPhysicsObjectBox3D::DetachShadowController()
{
	CShadowControllerBox3D *pShadow = m_pShadow;
	m_pShadow = NULL;
	return pShadow;
}

void CPhysicsObjectBox3D::AttachShadowController( CShadowControllerBox3D *pShadow )
{
	m_pShadow = pShadow;
}

void CPhysicsObjectBox3D::SetShadow( float maxSpeed, float maxAngularSpeed, bool allowPhysicsMovement, bool allowPhysicsRotation )
{
	EnsureShadowController( allowPhysicsMovement, allowPhysicsRotation )->MaxSpeed( maxSpeed, maxAngularSpeed );
}

void CPhysicsObjectBox3D::UpdateShadow( const Vector &targetPosition, const QAngle &targetAngles, bool tempDisableGravity, float timeOffset )
{
	// IVP toggles gravity itself while the shadow asks for it (not for
	// shadows that never translate, which have gravity off already).
	if ( tempDisableGravity != m_shadowTempGravityDisable )
	{
		m_shadowTempGravityDisable = tempDisableGravity;
		if ( !m_pShadow || m_pShadow->AllowsTranslation() )
			EnableGravity( !m_shadowTempGravityDisable );
	}
	if ( m_pShadow )
		m_pShadow->Update( targetPosition, targetAngles, timeOffset );
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
	if ( m_isTrigger )
		return;
	if ( m_pShadow )
		m_pShadow->UseShadowMaterial( false );
	EnableDrag( false );
	EnableGravity( false );
	// Box3D sensors cannot be toggled on a shape: rebuild the shapes as
	// sensors. A trigger no longer collides, and reports the objects that
	// overlap it (IVP's phantom).
	m_isTrigger = true;
	DestroyShapes();
	CreateShapes();
	ApplyFilter();
}

void CPhysicsObjectBox3D::RemoveTrigger()
{
	if ( !m_isTrigger )
		return;
	m_isTrigger = false;
	m_pEnv->TriggerRemoved( this );
	DestroyShapes();
	CreateShapes();
	ApplyFilter();
}

void CPhysicsObjectBox3D::BecomeHinged( int localAxis )
{
	if ( localAxis < 0 || localAxis > 2 )
		return;
	m_hingeAxis = localAxis;
	if ( !IsMoveable() )
		return;
	// IVP gives the other two axes near-infinite inertia; locking them is the
	// Box3D equivalent. Translation stays free.
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
	Msg( "Damping %.3e linear, %.3e angular\n", m_speedDamping, m_rotDamping );
	Msg( "Linear Drag: %.2f, %.2f, %.2f (factor %.2f)\n", m_dragBasis.x, m_dragBasis.y, m_dragBasis.z, m_dragCoefficient );
	Msg( "Angular Drag: %.2f, %.2f, %.2f (factor %.2f)\n", m_angDragBasis.x, m_angDragBasis.y, m_angDragBasis.z, m_angDragCoefficient );
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
