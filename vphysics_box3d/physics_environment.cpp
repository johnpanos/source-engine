//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics environment backed by a Box3D world (RFC 0004 B2/B5/C).
//
//=============================================================================//
#include "physics_environment.h"

#include <math.h>
#include <string.h>

#include "box3d/box3d.h"
#include "box3d_convert.h"
#include "cmodel.h"
#include "gametrace.h"
#include "physics_collision.h"
#include "physics_constraint.h"
#include "physics_fluid.h"
#include "physics_object.h"
#include "physics_vehicle.h"
#include "tier0/dbg.h"
#include "vstdlib/jobthread.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const float kDefaultTimestep = 0.015f;	// IVP's default PSI (DEFAULT_TICK_INTERVAL)
const int kSubSteps = 4;
const int kMaxStepsPerSimulate = 8;
const float kMetersPerInch = 0.0254f;
// IVP forgets a pair's last impact after a second (CPhysicsListenerCollision).
const float kImpactPairMemory = 1.0f;

// Box3D's worker tasks on the caller's thread pool (RFC 0013 P2). Box3D tasks
// are leaves: none enqueues or waits for another, and the thread that calls
// b3World_Step orchestrates the solver itself, so a step completes even when
// no pool thread is free. Finishing a task waits through the pool's
// YieldWait, which runs that task on the waiting thread if no worker has
// started it and never runs unrelated pool work.
void *EnqueuePoolTask( b3TaskCallback *pTask, void *pTaskContext, void *pUserContext, const char * )
{
	IThreadPool *pPool = static_cast<IThreadPool *>( pUserContext );
	CJob *pJob = pPool->QueueCall( pTask, pTaskContext );
	if ( !pJob )
	{
		// NULL tells Box3D the task already ran here.
		pTask( pTaskContext );
		return NULL;
	}
	return pJob;
}

void FinishPoolTask( void *pUserTask, void *pUserContext )
{
	CJob *pJob = static_cast<CJob *>( pUserTask );
	pJob->WaitForFinish( TT_INFINITE, static_cast<IThreadPool *>( pUserContext ) );
	pJob->Release();
}

CPhysicsObjectBox3D *ObjectOf( b3ShapeId shape )
{
	return b3Shape_IsValid( shape ) ? (CPhysicsObjectBox3D *)b3Shape_GetUserData( shape ) : NULL;
}

// Contact geometry handed to collision and touch callbacks as pInternalData.
class CCollisionDataBox3D : public IPhysicsCollisionData
{
public:
	CCollisionDataBox3D( const Vector &normal, const Vector &point, const Vector &speed )
		: m_normal( normal ), m_point( point ), m_speed( speed ) {}
	virtual void GetSurfaceNormal( Vector &out ) { out = m_normal; }
	virtual void GetContactPoint( Vector &out ) { out = m_point; }
	virtual void GetContactSpeed( Vector &out ) { out = m_speed; }

private:
	Vector m_normal;
	Vector m_point;
	Vector m_speed;
};

// Default overlay so GetDebugOverlay never returns NULL (as IVP).
class CDebugOverlayBox3D : public IVPhysicsDebugOverlay
{
public:
	virtual void AddEntityTextOverlay( int, int, float, int, int, int, int, const char *, ... ) {}
	virtual void AddBoxOverlay( const Vector &, const Vector &, const Vector &, QAngle const &, int, int, int, int, float ) {}
	virtual void AddTriangleOverlay( const Vector &, const Vector &, const Vector &, int, int, int, int, bool, float ) {}
	virtual void AddLineOverlay( const Vector &, const Vector &, int, int, int, bool, float ) {}
	virtual void AddTextOverlay( const Vector &, float, const char *, ... ) {}
	virtual void AddTextOverlay( const Vector &, int, float, const char *, ... ) {}
	virtual void AddScreenTextOverlay( float, float, float, int, int, int, int, const char * ) {}
	virtual void AddSweptBoxOverlay( const Vector &, const Vector &, const Vector &, const Vector &, const QAngle &, int, int, int, int, float ) {}
	virtual void AddTextOverlayRGB( const Vector &, int, float, float, float, float, float, const char *, ... ) {}
};

CDebugOverlayBox3D s_defaultDebugOverlay;

// IVP's material manager combines surfaces by product, clamped to [0, 1]
// (IVP_Material_Manager / CIVPMaterialManager), not Box3D's geometric-mean
// friction and maximum restitution.
//
// IVP's friction solver limits each contact's friction impulse by the pressure
// its gap spring measured, which settles about 5% below the true normal
// force: a sliding object decelerates at 0.95 * mu * g for every mass and
// material pair measured (1 to 1000 kg, ice to metal). The factor keeps
// pushed and sliding props moving as far as they do on IVP.
const float kIVPFrictionScale = 0.95f;

float MixFriction( float frictionA, uint64_t, float frictionB, uint64_t )
{
	return clamp( frictionA * frictionB, 0.0f, 1.0f ) * kIVPFrictionScale;
}

// The elasticity product is the fraction of normal-speed *energy* an impact
// keeps (IVP_Impact_Solver::do_impact gives back speed * sqrt(conservation)),
// so the velocity restitution is its square root: metal on concrete is
// sqrt(0.2 * 0.2) = 0.2, not 0.04.
float MixRestitution( float restitutionA, uint64_t, float restitutionB, uint64_t )
{
	return sqrtf( clamp( restitutionA * restitutionB, 0.0f, 1.0f ) );
}

// World-space contact point and normal (A to B) of a Box3D contact.
bool ContactGeometry( const b3ContactData &data, Vector *pPoint, Vector *pNormal )
{
	if ( data.manifoldCount <= 0 || data.manifolds[0].pointCount <= 0 )
		return false;
	const b3Manifold &manifold = data.manifolds[0];
	*pNormal = FromB3( manifold.normal );
	Vector centerA = FromB3( b3Body_GetWorldCenter( b3Shape_GetBody( data.shapeIdA ) ) );
	Vector point( 0, 0, 0 );
	for ( int p = 0; p < manifold.pointCount; p++ )
		point += centerA + FromB3( manifold.points[p].anchorA );
	*pPoint = point / (float)manifold.pointCount;
	return true;
}
}

CPhysicsEnvironmentBox3D::CPhysicsEnvironmentBox3D( int workerCount, IThreadPool *pThreadPool )
    : m_workerCount( workerCount > 1 && pThreadPool ? workerCount : 1 ),
      m_pThreadPool( pThreadPool ), m_simulateThread( ThreadGetCurrentId() ), m_solverCalls( 0 ),
      m_solverCallsOffCaller( 0 ), m_stepSeconds( 0.0 ), m_preStepSeconds( 0.0 ),
      m_postStepSeconds( 0.0 ), m_airDensity( 2.0f ), m_timestep( kDefaultTimestep ),
      m_timeAccumulator( 0.0f ), m_simulationTime( 0.0f ), m_stepCount( 0 ),
      m_inSimulation( false ), m_quickDelete( false ), m_queueDeleteObject( false ),
      m_enableConstraintNotify( false ), m_pSolver( NULL ), m_pCollisionEvents( NULL ),
      m_pObjectEvents( NULL ), m_pConstraintEvents( NULL ),
      m_pDebugOverlay( &s_defaultDebugOverlay )
{
	m_gravity.Init();
	memset( &m_stats, 0, sizeof( m_stats ) );
	memset( &m_lastProfile, 0, sizeof( m_lastProfile ) );
	m_performance.Defaults();

	b3WorldDef def = b3DefaultWorldDef();
	def.gravity = b3Vec3_zero;
	def.enableSleep = true;
	def.enableContinuous = true;
	// The worker count is fixed here: Box3D sizes its per-worker state from
	// it. Several workers run their tasks on the caller's pool; Box3D's own
	// scheduler (which would start threads per world) is never used.
	def.workerCount = m_workerCount;
	if ( m_workerCount > 1 )
	{
		def.enqueueTask = EnqueuePoolTask;
		def.finishTask = FinishPoolTask;
		def.userTaskContext = m_pThreadPool;
	}
	def.maximumLinearSpeed = m_performance.maxVelocity;
	def.frictionCallback = MixFriction;
	def.restitutionCallback = MixRestitution;
	def.userData = this;
	m_world = b3CreateWorld( &def );
	b3World_SetCustomFilterCallback( m_world, CustomFilter, this );
	b3World_SetPreSolveCallback( m_world, PreSolve, this );
	m_contactRecycleDistance = b3World_GetContactRecycleDistance( m_world );
}

CPhysicsEnvironmentBox3D::~CPhysicsEnvironmentBox3D()
{
	// No callbacks during shutdown; game code normally destroys its objects
	// first, anything left is released here.
	m_pSolver = NULL;
	m_pCollisionEvents = NULL;
	m_pObjectEvents = NULL;
	m_pConstraintEvents = NULL;
	m_quickDelete = true;
	m_constraints.PurgeAndDeleteElements();
	m_constraintGroups.PurgeAndDeleteElements();
	m_motionControllers.PurgeAndDeleteElements();
	m_playerControllers.PurgeAndDeleteElements();
	// Vehicles destroy their wheel objects.
	m_vehicles.PurgeAndDeleteElements();
	m_springs.PurgeAndDeleteElements();
	m_fluids.PurgeAndDeleteElements();
	ClearDeadObjects();
	for ( int i = m_objects.Count() - 1; i >= 0; i-- )
		delete m_objects[i];
	m_objects.RemoveAll();
	b3DestroyWorld( m_world );
}

void CPhysicsEnvironmentBox3D::SetDebugOverlay( CreateInterfaceFn debugOverlayFactory )
{
	m_pDebugOverlay = NULL;
	if ( debugOverlayFactory )
		m_pDebugOverlay = (IVPhysicsDebugOverlay *)debugOverlayFactory( VPHYSICS_DEBUG_OVERLAY_INTERFACE_VERSION, NULL );
	if ( !m_pDebugOverlay )
		m_pDebugOverlay = &s_defaultDebugOverlay;
}

IVPhysicsDebugOverlay *CPhysicsEnvironmentBox3D::GetDebugOverlay( void ) { return m_pDebugOverlay; }

void CPhysicsEnvironmentBox3D::SetGravity( const Vector &gravityVector )
{
	m_gravity = gravityVector;
	b3World_SetGravity( m_world, ToB3( gravityVector ) );
}

void CPhysicsEnvironmentBox3D::GetGravity( Vector *pGravityVector ) const { *pGravityVector = m_gravity; }

void CPhysicsEnvironmentBox3D::SetAirDensity( float density ) { m_airDensity = density; }
float CPhysicsEnvironmentBox3D::GetAirDensity( void ) const { return m_airDensity; }

//-----------------------------------------------------------------------------
// Objects and their lifetime
//-----------------------------------------------------------------------------
IPhysicsObject *CPhysicsEnvironmentBox3D::TrackObject( CPhysicsObjectBox3D *pObject )
{
	if ( pObject )
		m_objects.AddToTail( pObject );
	return pObject;
}

bool CPhysicsEnvironmentBox3D::IsLive( const IPhysicsObject *pObject ) const
{
	if ( !pObject )
		return false;
	const CPhysicsObjectBox3D *pBox = static_cast<const CPhysicsObjectBox3D *>( pObject );
	return !pBox->IsMarkedForDelete() && pBox->GetEnvironment() == this;
}

IPhysicsObject *CPhysicsEnvironmentBox3D::CreatePolyObject( const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, objectparams_t *pParams )
{
	if ( !pCollisionModel )
		return NULL;
	return TrackObject( new CPhysicsObjectBox3D( this, pCollisionModel, 0.0f, materialIndex, position, angles, pParams, false ) );
}

IPhysicsObject *CPhysicsEnvironmentBox3D::CreatePolyObjectStatic( const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, objectparams_t *pParams )
{
	if ( !pCollisionModel )
		return NULL;
	return TrackObject( new CPhysicsObjectBox3D( this, pCollisionModel, 0.0f, materialIndex, position, angles, pParams, true ) );
}

IPhysicsObject *CPhysicsEnvironmentBox3D::CreateSphereObject( float radius, int materialIndex, const Vector &position, const QAngle &angles, objectparams_t *pParams, bool isStatic )
{
	return TrackObject( new CPhysicsObjectBox3D( this, NULL, radius, materialIndex, position, angles, pParams, isStatic ) );
}

// Everything that references an object drops it before the object goes away
// (or leaves for another environment). Constraints become inert as IVP's do
// when their object is deleted, and are reported when notification is on.
void CPhysicsEnvironmentBox3D::DetachObject( CPhysicsObjectBox3D *pObject, bool notifyConstraints )
{
	for ( int i = 0; i < m_constraints.Count(); i++ )
	{
		CConstraintBox3D *pConstraint = m_constraints[i];
		if ( !pConstraint->Links( pObject ) )
			continue;
		pConstraint->ObjectDestroyed();
		if ( notifyConstraints && m_enableConstraintNotify && m_pConstraintEvents )
			m_pConstraintEvents->ConstraintBroken( pConstraint );
	}
	for ( int i = 0; i < m_springs.Count(); i++ )
	{
		if ( m_springs[i]->Links( pObject ) )
			m_springs[i]->ObjectDestroyed();
	}
	for ( int i = 0; i < m_motionControllers.Count(); i++ )
		m_motionControllers[i]->ObjectDestroyed( pObject );
	for ( int i = 0; i < m_playerControllers.Count(); i++ )
		m_playerControllers[i]->ObjectDestroyed( pObject );
	for ( int i = 0; i < m_vehicles.Count(); i++ )
		m_vehicles[i]->ObjectDestroyed( pObject );
	for ( int i = m_deletedPairs.Count() - 1; i >= 0; i-- )
	{
		if ( m_deletedPairs[i].pA == pObject || m_deletedPairs[i].pB == pObject )
			m_deletedPairs.FastRemove( i );
	}
	for ( int i = m_triggerOverlaps.Count() - 1; i >= 0; i-- )
	{
		if ( m_triggerOverlaps[i].pTrigger == pObject || m_triggerOverlaps[i].pObject == pObject )
			m_triggerOverlaps.FastRemove( i );
	}
	for ( int i = m_impactPairs.Count() - 1; i >= 0; i-- )
	{
		if ( m_impactPairs[i].pA == pObject || m_impactPairs[i].pB == pObject )
			m_impactPairs.FastRemove( i );
	}
}

void CPhysicsEnvironmentBox3D::DestroyObject( IPhysicsObject *pObject )
{
	if ( !pObject )
		return;
	int index = m_objects.Find( pObject );
	if ( index == m_objects.InvalidIndex() )
	{
		// Deleted twice or not ours (IVP asserts).
		return;
	}
	m_objects.Remove( index );
	CPhysicsObjectBox3D *pBox = ToBox3D( pObject );
	pBox->SetCallbackFlags( pBox->GetCallbackFlags() | CALLBACK_MARKED_FOR_DELETE );
	if ( m_inSimulation || m_queueDeleteObject )
	{
		// Never delete while simulating (callbacks run inside the step).
		m_deadObjects.AddToTail( pObject );
		return;
	}
	DetachObject( pBox, true );
	delete pBox;
}

void CPhysicsEnvironmentBox3D::ClearDeadObjects()
{
	// Deleting may run game callbacks that queue more deaths; drain them.
	while ( m_deadObjects.Count() )
	{
		CUtlVector<IPhysicsObject *> dead;
		dead.Swap( m_deadObjects );
		for ( int i = 0; i < dead.Count(); i++ )
		{
			CPhysicsObjectBox3D *pBox = ToBox3D( dead[i] );
			DetachObject( pBox, true );
			delete pBox;
		}
	}
}

void CPhysicsEnvironmentBox3D::CleanupDeleteList( void ) { ClearDeadObjects(); }
void CPhysicsEnvironmentBox3D::EnableDeleteQueue( bool enable ) { m_queueDeleteObject = enable; }

bool CPhysicsEnvironmentBox3D::IsCollisionModelUsed( CPhysCollide *pCollide ) const
{
	for ( int i = 0; i < m_deadObjects.Count(); i++ )
	{
		if ( m_deadObjects[i]->GetCollide() == pCollide )
			return true;
	}
	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		if ( m_objects[i]->GetCollide() == pCollide )
			return true;
	}
	return false;
}

bool CPhysicsEnvironmentBox3D::TransferObject( IPhysicsObject *pObject, IPhysicsEnvironment *pDestinationEnvironment )
{
	int index = m_objects.Find( pObject );
	if ( index == m_objects.InvalidIndex() || !pDestinationEnvironment || pDestinationEnvironment == this )
		return false;
	CPhysicsObjectBox3D *pBox = ToBox3D( pObject );
	if ( pBox->IsMarkedForDelete() )
		return false;
	CPhysicsEnvironmentBox3D *pDest = static_cast<CPhysicsEnvironmentBox3D *>( pDestinationEnvironment );

	// As IVP: controllers ride along silently; constraints and other links in
	// this environment end as if the object were deleted here.
	CShadowControllerBox3D *pShadow = pBox->DetachShadowController();
	CPlayerControllerBox3D *pPlayer = FindPlayerController( pObject );
	if ( pPlayer )
		RemovePlayerController( pPlayer );
	DetachObject( pBox, true );
	m_objects.Remove( index );

	pBox->MoveToEnvironment( pDest );
	pDest->m_objects.AddToTail( pObject );
	// Put it in the destination's active set right away to avoid a hitch.
	pBox->Wake();

	if ( pShadow )
		pBox->AttachShadowController( pShadow );
	if ( pPlayer )
		pDest->AddPlayerController( pPlayer );
	return true;
}

unsigned int CPhysicsEnvironmentBox3D::GetObjectSerializeSize( IPhysicsObject *pObject ) const
{
	return sizeof( CPhysicsObjectStateBox3D );
}

void CPhysicsEnvironmentBox3D::SerializeObjectToBuffer( IPhysicsObject *pObject, unsigned char *pBuffer, unsigned int bufferSize )
{
	if ( !pObject || !pBuffer || bufferSize < sizeof( CPhysicsObjectStateBox3D ) )
		return;
	CPhysicsObjectStateBox3D state;
	ToBox3D( pObject )->WriteState( state );
	memcpy( pBuffer, &state, sizeof( state ) );
}

IPhysicsObject *CPhysicsEnvironmentBox3D::UnserializeObjectFromBuffer( void *pGameData, unsigned char *pBuffer, unsigned int bufferSize, bool enableCollisions )
{
	if ( !pBuffer || bufferSize < sizeof( CPhysicsObjectStateBox3D ) )
		return NULL;
	CPhysicsObjectStateBox3D state;
	memcpy( &state, pBuffer, sizeof( state ) );
	// Shadow controllers are not carried by this path (IVP drops them too).
	return TrackObject( CPhysicsObjectBox3D::CreateFromState( this, pGameData, state, enableCollisions ) );
}

//-----------------------------------------------------------------------------
// Fluids and springs
//-----------------------------------------------------------------------------
IPhysicsFluidController *CPhysicsEnvironmentBox3D::CreateFluidController( IPhysicsObject *pFluidObject, fluidparams_t *pParams )
{
	if ( !pFluidObject || !pParams )
		return NULL;
	CPhysicsFluidControllerBox3D *pFluid = new CPhysicsFluidControllerBox3D( ToBox3D( pFluidObject ), *pParams );
	m_fluids.AddToTail( pFluid );
	return pFluid;
}

void CPhysicsEnvironmentBox3D::DestroyFluidController( IPhysicsFluidController *pFluid )
{
	CPhysicsFluidControllerBox3D *pBox = static_cast<CPhysicsFluidControllerBox3D *>( pFluid );
	if ( pBox && m_fluids.FindAndRemove( pBox ) )
		delete pBox;
}

IPhysicsSpring *CPhysicsEnvironmentBox3D::CreateSpring( IPhysicsObject *pObjectStart, IPhysicsObject *pObjectEnd, springparams_t *pParams )
{
	if ( !pObjectStart || !pObjectEnd || !pParams )
		return NULL;
	CPhysicsSpringBox3D *pSpring = new CPhysicsSpringBox3D( ToBox3D( pObjectStart ), ToBox3D( pObjectEnd ), *pParams );
	m_springs.AddToTail( pSpring );
	return pSpring;
}

void CPhysicsEnvironmentBox3D::DestroySpring( IPhysicsSpring *pSpring )
{
	CPhysicsSpringBox3D *pBox = static_cast<CPhysicsSpringBox3D *>( pSpring );
	if ( pBox && m_springs.FindAndRemove( pBox ) )
		delete pBox;
}

//-----------------------------------------------------------------------------
// Constraints
//-----------------------------------------------------------------------------
CConstraintBox3D *CPhysicsEnvironmentBox3D::TrackConstraint( IPhysicsObject *pReference, IPhysicsObject *pAttached,
	IPhysicsConstraintGroup *pGroup, int type, const constraint_breakableparams_t &breakable )
{
	if ( !pReference || !pAttached )
		return NULL;
	CConstraintBox3D *pConstraint = new CConstraintBox3D( this, pReference, pAttached,
		static_cast<CConstraintGroupBox3D *>( pGroup ), (ConstraintTypeBox3D_t)type, breakable );
	m_constraints.AddToTail( pConstraint );
	return pConstraint;
}

IPhysicsConstraint *CPhysicsEnvironmentBox3D::CreateFixedConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_fixedparams_t &fixed )
{
	CConstraintBox3D *pConstraint = TrackConstraint( pReferenceObject, pAttachedObject, pGroup, CONSTRAINT_BOX3D_FIXED, fixed.constraint );
	if ( pConstraint )
		pConstraint->InitFixed( fixed );
	return pConstraint;
}

IPhysicsConstraint *CPhysicsEnvironmentBox3D::CreateRagdollConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_ragdollparams_t &ragdoll )
{
	CConstraintBox3D *pConstraint = TrackConstraint( pReferenceObject, pAttachedObject, pGroup, CONSTRAINT_BOX3D_RAGDOLL, ragdoll.constraint );
	if ( pConstraint )
		pConstraint->InitRagdoll( ragdoll );
	return pConstraint;
}

IPhysicsConstraint *CPhysicsEnvironmentBox3D::CreateHingeConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_hingeparams_t &hinge )
{
	CConstraintBox3D *pConstraint = TrackConstraint( pReferenceObject, pAttachedObject, pGroup, CONSTRAINT_BOX3D_HINGE, hinge.constraint );
	if ( pConstraint )
		pConstraint->InitHinge( constraint_limitedhingeparams_t( hinge ) );
	return pConstraint;
}

IPhysicsConstraint *CPhysicsEnvironmentBox3D::CreateSlidingConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_slidingparams_t &sliding )
{
	CConstraintBox3D *pConstraint = TrackConstraint( pReferenceObject, pAttachedObject, pGroup, CONSTRAINT_BOX3D_SLIDING, sliding.constraint );
	if ( pConstraint )
		pConstraint->InitSliding( sliding );
	return pConstraint;
}

IPhysicsConstraint *CPhysicsEnvironmentBox3D::CreateBallsocketConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_ballsocketparams_t &ballsocket )
{
	CConstraintBox3D *pConstraint = TrackConstraint( pReferenceObject, pAttachedObject, pGroup, CONSTRAINT_BOX3D_BALLSOCKET, ballsocket.constraint );
	if ( pConstraint )
		pConstraint->InitBallsocket( ballsocket );
	return pConstraint;
}

IPhysicsConstraint *CPhysicsEnvironmentBox3D::CreatePulleyConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_pulleyparams_t &pulley )
{
	CConstraintBox3D *pConstraint = TrackConstraint( pReferenceObject, pAttachedObject, pGroup, CONSTRAINT_BOX3D_PULLEY, pulley.constraint );
	if ( pConstraint )
		pConstraint->InitPulley( pulley );
	return pConstraint;
}

IPhysicsConstraint *CPhysicsEnvironmentBox3D::CreateLengthConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_lengthparams_t &length )
{
	CConstraintBox3D *pConstraint = TrackConstraint( pReferenceObject, pAttachedObject, pGroup, CONSTRAINT_BOX3D_LENGTH, length.constraint );
	if ( pConstraint )
		pConstraint->InitLength( length );
	return pConstraint;
}

void CPhysicsEnvironmentBox3D::DestroyConstraint( IPhysicsConstraint *pConstraint )
{
	CConstraintBox3D *pBox = static_cast<CConstraintBox3D *>( pConstraint );
	if ( !pBox || !m_constraints.FindAndRemove( pBox ) )
		return;
	// As IVP: unless quick-deleting, destroying a constraint wakes its objects.
	if ( !m_quickDelete )
	{
		if ( pBox->GetReferenceObject() )
			pBox->GetReferenceObject()->Wake();
		if ( pBox->GetAttachedObject() )
			pBox->GetAttachedObject()->Wake();
	}
	delete pBox;
}

IPhysicsConstraintGroup *CPhysicsEnvironmentBox3D::CreateConstraintGroup( const constraint_groupparams_t &params )
{
	CConstraintGroupBox3D *pGroup = new CConstraintGroupBox3D( params );
	m_constraintGroups.AddToTail( pGroup );
	return pGroup;
}

void CPhysicsEnvironmentBox3D::DestroyConstraintGroup( IPhysicsConstraintGroup *pGroup )
{
	CConstraintGroupBox3D *pBox = static_cast<CConstraintGroupBox3D *>( pGroup );
	if ( pBox && m_constraintGroups.FindAndRemove( pBox ) )
		delete pBox;
}

void CPhysicsEnvironmentBox3D::CheckConstraintBreaks( float dt )
{
	// Handlers may destroy constraints (and objects); walk a snapshot.
	CUtlVector<CConstraintBox3D *> constraints;
	constraints.CopyArray( m_constraints.Base(), m_constraints.Count() );
	for ( int i = 0; i < constraints.Count(); i++ )
	{
		if ( m_constraints.Find( constraints[i] ) == m_constraints.InvalidIndex() )
			continue;
		if ( constraints[i]->CheckBreak( dt ) && m_pConstraintEvents )
			m_pConstraintEvents->ConstraintBroken( constraints[i] );
	}
}

void CPhysicsEnvironmentBox3D::EnableConstraintNotify( bool bEnable ) { m_enableConstraintNotify = bEnable; }

//-----------------------------------------------------------------------------
// Controllers
//-----------------------------------------------------------------------------
IPhysicsShadowController *CPhysicsEnvironmentBox3D::CreateShadowController( IPhysicsObject *pObject, bool allowTranslation, bool allowRotation )
{
	// The shadow controller is owned by the object it drives, so both this path
	// and IPhysicsObject::SetShadow resolve to the same controller.
	return ToBox3D( pObject )->EnsureShadowController( allowTranslation, allowRotation );
}

void CPhysicsEnvironmentBox3D::DestroyShadowController( IPhysicsShadowController *pController )
{
	if ( !pController )
		return;
	CPhysicsObjectBox3D *pObject = static_cast<CShadowControllerBox3D *>( pController )->GetObject();
	if ( pObject && pObject->GetShadowController() == pController )
		pObject->RemoveShadowController();
	else
		delete pController;
}

void CPhysicsEnvironmentBox3D::AddPlayerController( CPlayerControllerBox3D *pController )
{
	if ( m_playerControllers.Find( pController ) == m_playerControllers.InvalidIndex() )
		m_playerControllers.AddToTail( pController );
}

void CPhysicsEnvironmentBox3D::RemovePlayerController( CPlayerControllerBox3D *pController )
{
	m_playerControllers.FindAndRemove( pController );
}

CPlayerControllerBox3D *CPhysicsEnvironmentBox3D::FindPlayerController( IPhysicsObject *pObject ) const
{
	for ( int i = m_playerControllers.Count() - 1; i >= 0; --i )
	{
		if ( m_playerControllers[i]->GetObject() == pObject )
			return m_playerControllers[i];
	}
	return NULL;
}

IPhysicsPlayerController *CPhysicsEnvironmentBox3D::CreatePlayerController( IPhysicsObject *pObject )
{
	CPlayerControllerBox3D *pController = new CPlayerControllerBox3D( ToBox3D( pObject ) );
	AddPlayerController( pController );
	return pController;
}

void CPhysicsEnvironmentBox3D::DestroyPlayerController( IPhysicsPlayerController *pController )
{
	CPlayerControllerBox3D *pBox = static_cast<CPlayerControllerBox3D *>( pController );
	if ( !pBox )
		return;
	RemovePlayerController( pBox );
	delete pBox;
}

IPhysicsMotionController *CPhysicsEnvironmentBox3D::CreateMotionController( IMotionEvent *pHandler )
{
	CMotionControllerBox3D *pController = new CMotionControllerBox3D( pHandler );
	m_motionControllers.AddToTail( pController );
	return pController;
}

void CPhysicsEnvironmentBox3D::DestroyMotionController( IPhysicsMotionController *pController )
{
	CMotionControllerBox3D *pBox = static_cast<CMotionControllerBox3D *>( pController );
	if ( pBox && m_motionControllers.FindAndRemove( pBox ) )
		delete pBox;
}

IPhysicsVehicleController *CPhysicsEnvironmentBox3D::CreateVehicleController( IPhysicsObject *pVehicleBodyObject, const vehicleparams_t &params, unsigned int nVehicleType, IPhysicsGameTrace *pGameTrace )
{
	if ( !pVehicleBodyObject )
		return NULL;
	CVehicleControllerBox3D *pController = new CVehicleControllerBox3D( this, params, nVehicleType, pGameTrace );
	pController->InitCarSystem( ToBox3D( pVehicleBodyObject ) );
	m_vehicles.AddToTail( pController );
	return pController;
}

void CPhysicsEnvironmentBox3D::DestroyVehicleController( IPhysicsVehicleController *pController )
{
	CVehicleControllerBox3D *pBox = static_cast<CVehicleControllerBox3D *>( pController );
	if ( pBox && m_vehicles.FindAndRemove( pBox ) )
		delete pBox;
}

//-----------------------------------------------------------------------------
// Simulation
//-----------------------------------------------------------------------------
bool CPhysicsEnvironmentBox3D::CustomFilter( b3ShapeId shapeIdA, b3ShapeId shapeIdB, void *pContext )
{
	CPhysicsEnvironmentBox3D *pEnv = static_cast<CPhysicsEnvironmentBox3D *>( pContext );
	return pEnv->PairAllowed( ObjectOf( shapeIdA ), ObjectOf( shapeIdB ) );
}

bool CPhysicsEnvironmentBox3D::PairAllowed( CPhysicsObjectBox3D *pA, CPhysicsObjectBox3D *pB ) const
{
	if ( !pA || !pB || pA == pB )
		return false;
	if ( !pA->IsCollisionEnabled() || !pB->IsCollisionEnabled() )
		return false;
	// IVP's CCollisionSolver::check_objects_for_collision_detection: an object
	// being re-enabled never collides with one about to be deleted.
	unsigned int flagsA = pA->GetCallbackFlags(), flagsB = pB->GetCallbackFlags();
	if ( ( flagsA & CALLBACK_ENABLING_COLLISION ) && ( flagsB & CALLBACK_MARKED_FOR_DELETE ) )
		return false;
	if ( ( flagsB & CALLBACK_ENABLING_COLLISION ) && ( flagsA & CALLBACK_MARKED_FOR_DELETE ) )
		return false;
	// The game's collision rules (portal environments, player/prop filters).
	if ( !m_pSolver )
		return true;
	if ( m_workerCount > 1 )
	{
		std::lock_guard<std::mutex> lock( m_solverMutex );
		m_solverCalls++;
		if ( ThreadGetCurrentId() != m_simulateThread )
			m_solverCallsOffCaller++;
		return m_pSolver->ShouldCollide( pA, pB, pA->GetGameData(), pB->GetGameData() ) != 0;
	}
	m_solverCalls++;
	return m_pSolver->ShouldCollide( pA, pB, pA->GetGameData(), pB->GetGameData() ) != 0;
}

// Box3D consults the collision rules when a pair's contact is created and
// keeps the contact while the shapes' bounds overlap. IVP likewise keeps its
// contact until the game deletes it through a friction snapshot, after which
// the pair collides again only if the rules allow it. The deleted contact is
// held off (pre-solve) for the next step, then for as long as the rules say
// no.
void CPhysicsEnvironmentBox3D::DeleteContactPair( CPhysicsObjectBox3D *pA, CPhysicsObjectBox3D *pB, bool wake )
{
	if ( !pA || !pB || pA == pB )
		return;
	int index = FindDeletedPair( pA, pB );
	if ( index >= 0 )
	{
		m_deletedPairs[index].fresh = true;
		m_deletedPairs[index].disabled = true;
		return;
	}
	DeletedPair_t pair = { pA, pB, true, true };
	m_deletedPairs.AddToTail( pair );

	// As IVP, only the partner is woken.
	if ( wake && !pB->IsStatic() )
		pB->Wake();
}

int CPhysicsEnvironmentBox3D::FindDeletedPair(
    const CPhysicsObjectBox3D *pA, const CPhysicsObjectBox3D *pB ) const
{
	for ( int i = 0; i < m_deletedPairs.Count(); i++ )
	{
		const DeletedPair_t &pair = m_deletedPairs[i];
		if ( ( pair.pA == pA && pair.pB == pB ) || ( pair.pA == pB && pair.pB == pA ) )
			return i;
	}
	return -1;
}

namespace
{
bool CollectShape( b3ShapeId shapeId, void *pContext )
{
	static_cast<CUtlVector<b3ShapeId> *>( pContext )->AddToTail( shapeId );
	return true;
}

// Shapes that overlap one of a body's convex shapes where it is now.
void OverlappingShapes( b3WorldId world, b3ShapeId shape, CUtlVector<b3ShapeId> &out )
{
	b3WorldTransform xform = b3Body_GetTransform( b3Shape_GetBody( shape ) );
	b3Vec3 points[B3_MAX_SHAPE_CAST_POINTS];
	b3ShapeProxy proxy;
	memset( &proxy, 0, sizeof( proxy ) );
	proxy.points = points;
	switch ( b3Shape_GetType( shape ) )
	{
	case b3_hullShape:
	{
		const b3HullData *pHull = b3Shape_GetHull( shape );
		const b3Vec3 *pPoints = b3GetHullPoints( pHull );
		proxy.count = MIN( pHull->vertexCount, B3_MAX_SHAPE_CAST_POINTS );
		for ( int i = 0; i < proxy.count; i++ )
			points[i] = b3RotateVector( xform.q, pPoints[i] );
		break;
	}
	case b3_sphereShape:
	{
		b3Sphere sphere = b3Shape_GetSphere( shape );
		points[0] = b3RotateVector( xform.q, sphere.center );
		proxy.count = 1;
		proxy.radius = sphere.radius;
		break;
	}
	default:
		return;
	}
	b3World_OverlapShape( world, xform.p, &proxy, b3DefaultQueryFilter(), CollectShape, &out );
}
}

// Box3D consults the collision rules only when a pair's contact is created;
// IVP's recheck_collision_filter re-runs them on the object's pairs. A pair the
// rules now reject is held off (pre-solve) until they allow it again, as a
// deleted contact is. A dynamic object overlapping a partner the rules allow
// but lacking a contact with it (the rules rejected the pair when it formed)
// has its proxies re-paired; Box3D re-pairs a shape only by resetting its
// contacts, so this is limited to that case.
void CPhysicsEnvironmentBox3D::RecheckPairs( CPhysicsObjectBox3D *pObject )
{
	if ( !IsLive( pObject ) || !pObject->IsCollisionEnabled() )
		return;
	b3BodyId body = pObject->GetBody();

	// Candidates: every shape whose fat bounds can overlap the object's, so any
	// pair with a contact, touching or not.
	const float margin = 2.0f * B3_MAX_AABB_MARGIN;
	b3AABB bounds = b3AABB_Inflate( b3Body_ComputeAABB( body ), margin );
	CUtlVector<b3ShapeId> candidates;
	b3World_OverlapAABB( m_world, bounds, b3DefaultQueryFilter(), CollectShape, &candidates );

	CUtlVector<CPhysicsObjectBox3D *> allowed;
	for ( int i = 0; i < candidates.Count(); i++ )
	{
		CPhysicsObjectBox3D *pOther = ObjectOf( candidates[i] );
		if ( !pOther || pOther == pObject || !IsLive( pOther ) ||
		     b3Shape_IsSensor( candidates[i] ) )
			continue;
		if ( ( pObject->IsStatic() && pOther->IsStatic() ) || !pOther->IsCollisionEnabled() )
			continue;
		if ( allowed.Find( pOther ) != allowed.InvalidIndex() )
			continue;
		int index = FindDeletedPair( pObject, pOther );
		if ( PairAllowed( pObject, pOther ) )
		{
			allowed.AddToTail( pOther );
			if ( index >= 0 && !m_deletedPairs[index].fresh )
				m_deletedPairs.FastRemove( index );
		}
		else if ( index < 0 )
		{
			DeletedPair_t pair = { pObject, pOther, false, true };
			m_deletedPairs.AddToTail( pair );
		}
		else
		{
			m_deletedPairs[index].disabled = true;
		}
	}

	if ( pObject->IsStatic() || !allowed.Count() )
		return;

	CUtlVector<CPhysicsObjectBox3D *> touching;
	int capacity = b3Body_GetContactCapacity( body );
	if ( capacity > 0 )
	{
		CUtlVector<b3ContactData> contacts;
		contacts.SetCount( capacity );
		int count = b3Body_GetContactData( body, contacts.Base(), capacity );
		for ( int c = 0; c < count; c++ )
		{
			CPhysicsObjectBox3D *pA = ObjectOf( contacts[c].shapeIdA );
			touching.AddToTail( pA == pObject ? ObjectOf( contacts[c].shapeIdB ) : pA );
		}
	}

	CUtlVector<b3ShapeId> shapes, overlapping;
	shapes.SetCount( b3Body_GetShapeCount( body ) );
	if ( shapes.Count() )
		shapes.SetCount( b3Body_GetShapes( body, shapes.Base(), shapes.Count() ) );
	for ( int s = 0; s < shapes.Count(); s++ )
	{
		overlapping.RemoveAll();
		OverlappingShapes( m_world, shapes[s], overlapping );
		bool repair = false;
		for ( int i = 0; i < overlapping.Count() && !repair; i++ )
		{
			CPhysicsObjectBox3D *pOther = ObjectOf( overlapping[i] );
			if ( allowed.Find( pOther ) == allowed.InvalidIndex() ||
			     touching.Find( pOther ) != touching.InvalidIndex() )
				continue;
			int index = FindDeletedPair( pObject, pOther );
			repair = index < 0 || !m_deletedPairs[index].disabled;
		}
		if ( !repair )
			continue;
		// A filter change resets the shape's proxy, so the broadphase pairs it
		// again under the current rules.
		b3Filter filter = b3Shape_GetFilter( shapes[s] );
		b3Filter toggled = filter;
		toggled.groupIndex = filter.groupIndex == 0 ? 1 : 0;
		b3Shape_SetFilter( shapes[s], toggled, true );
		b3Shape_SetFilter( shapes[s], filter, true );
	}
}

void CPhysicsEnvironmentBox3D::UpdateDeletedPairs()
{
	// Recycled contacts skip the narrow phase, and with it the pre-solve that
	// holds a deleted contact off: recompute every contact while a deletion
	// is pending (Box3D recycles only with a positive recycle distance).
	struct Restore_t
	{
		CPhysicsEnvironmentBox3D *pEnv;
		~Restore_t()
		{
			float distance = pEnv->m_deletedPairs.Count() ? 0.0f : pEnv->m_contactRecycleDistance;
			if ( b3World_GetContactRecycleDistance( pEnv->m_world ) != distance )
				b3World_SetContactRecycleDistance( pEnv->m_world, distance );
		}
	} restore = { this };

	for ( int i = m_deletedPairs.Count() - 1; i >= 0; i-- )
	{
		DeletedPair_t &pair = m_deletedPairs[i];
		if ( !IsLive( pair.pA ) || !IsLive( pair.pB ) )
		{
			m_deletedPairs.FastRemove( i );
			continue;
		}
		if ( pair.fresh )
		{
			pair.fresh = false;
			pair.disabled = true;
			continue;
		}
		// IVP re-creates the contact once the rules allow the pair again.
		if ( PairAllowed( pair.pA, pair.pB ) )
			m_deletedPairs.FastRemove( i );
	}
}

bool CPhysicsEnvironmentBox3D::PreSolve( b3ShapeId shapeIdA, b3ShapeId shapeIdB, b3Pos point, b3Vec3 normal, void *pContext )
{
	CPhysicsEnvironmentBox3D *pEnv = static_cast<CPhysicsEnvironmentBox3D *>( pContext );
	if ( !pEnv->m_deletedPairs.Count() )
		return true;
	CPhysicsObjectBox3D *pA = ObjectOf( shapeIdA ), *pB = ObjectOf( shapeIdB );
	for ( int i = 0; i < pEnv->m_deletedPairs.Count(); i++ )
	{
		const DeletedPair_t &pair = pEnv->m_deletedPairs[i];
		if ( ( pair.pA == pA && pair.pB == pB ) || ( pair.pA == pB && pair.pB == pA ) )
			return !pair.disabled;
	}
	return true;
}

void CPhysicsEnvironmentBox3D::SetCollisionSolver( IPhysicsCollisionSolver *pSolver ) { m_pSolver = pSolver; }

// IVP runs its controllers by priority, highest first: buoyancy (1600),
// springs (1400), gravity with damping (1000), the player controller (501),
// shadow and motion controllers and drag (500), then constraints (400s). The
// order matters to the game: a controller that sets a velocity (the grab and
// player controllers) overrides this step's gravity and damping instead of
// having them applied on top.
void CPhysicsEnvironmentBox3D::PreStep( float dt )
{
	UpdateDeletedPairs();
	for ( int i = 0; i < m_fluids.Count(); i++ )
	{
		CPhysicsObjectBox3D *pFluidObject = m_fluids[i]->GetObject();
		for ( int k = 0; k < m_triggerOverlaps.Count(); k++ )
		{
			if ( m_triggerOverlaps[k].pTrigger == pFluidObject && IsLive( m_triggerOverlaps[k].pObject ) )
				m_fluids[i]->ApplyToObject( m_triggerOverlaps[k].pObject, dt );
		}
	}
	for ( int i = 0; i < m_springs.Count(); i++ )
		m_springs[i]->Simulate( dt );
	// Objects a controller drives this step.
	CUtlVector<IPhysicsObject *> controlled;
	for ( int i = 0; i < m_playerControllers.Count(); i++ )
	{
		if ( m_playerControllers[i]->GetObject() )
			controlled.AddToTail( m_playerControllers[i]->GetObject() );
	}
	for ( int i = 0; i < m_motionControllers.Count(); i++ )
	{
		int count = m_motionControllers[i]->CountObjects();
		int base = controlled.AddMultipleToTail( count );
		m_motionControllers[i]->GetObjects( controlled.Base() + base );
	}
	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		CPhysicsObjectBox3D *pObject = ToBox3D( m_objects[i] );
		bool isControlled = pObject->GetShadow() != NULL || controlled.Find( pObject ) != controlled.InvalidIndex();
		pObject->ApplyGravityAndDamping( dt, m_gravity, isControlled );
	}

	for ( int i = 0; i < m_playerControllers.Count(); i++ )
		m_playerControllers[i]->Simulate( dt );
	// Controllers may create or destroy objects; walk snapshots.
	CUtlVector<IPhysicsObject *> objects;
	objects.CopyArray( m_objects.Base(), m_objects.Count() );
	for ( int i = 0; i < objects.Count(); i++ )
	{
		if ( !IsLive( objects[i] ) )
			continue;
		CShadowControllerBox3D *pShadow = ToBox3D( objects[i] )->GetShadow();
		if ( pShadow )
			pShadow->Simulate( dt );
	}
	CUtlVector<CMotionControllerBox3D *> motion;
	motion.CopyArray( m_motionControllers.Base(), m_motionControllers.Count() );
	for ( int i = 0; i < motion.Count(); i++ )
	{
		if ( m_motionControllers.Find( motion[i] ) != m_motionControllers.InvalidIndex() )
			motion[i]->Simulate( dt );
	}
	for ( int i = 0; i < m_objects.Count(); i++ )
		ToBox3D( m_objects[i] )->ApplyDrag( dt, m_airDensity );
	for ( int i = 0; i < m_vehicles.Count(); i++ )
		m_vehicles[i]->Simulate( dt );
	for ( int i = 0; i < m_constraints.Count(); i++ )
		m_constraints[i]->PreStep( dt );

	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		CPhysicsObjectBox3D *pObject = ToBox3D( m_objects[i] );
		pObject->CommitVelocity();
		pObject->CapturePreStepVelocity();
		pObject->SetWasAwake( !pObject->IsAsleep() );
	}
}

float CPhysicsEnvironmentBox3D::PairDeltaTime( IPhysicsObject *pA, IPhysicsObject *pB )
{
	// IVP's deltaCollisionTime: time since this pair's last impact, or 1.0
	// for a first impact (or one forgotten after a second).
	for ( int i = m_impactPairs.Count() - 1; i >= 0; i-- )
	{
		ImpactPair_t &pair = m_impactPairs[i];
		if ( m_simulationTime - pair.lastTime > kImpactPairMemory )
		{
			m_impactPairs.FastRemove( i );
			continue;
		}
		if ( ( pair.pA == pA && pair.pB == pB ) || ( pair.pA == pB && pair.pB == pA ) )
		{
			float delta = m_simulationTime - pair.lastTime;
			pair.lastTime = m_simulationTime;
			return delta > 0.0f ? delta : 1.0f;
		}
	}
	ImpactPair_t pair = { pA, pB, m_simulationTime };
	m_impactPairs.AddToTail( pair );
	return 1.0f;
}

void CPhysicsEnvironmentBox3D::DispatchContactEvents( float dt )
{
	b3ContactEvents events = b3World_GetContactEvents( m_world );
	m_stats.collisionPairsCreated += events.beginCount;
	m_stats.collisionPairsDestroyed += events.endCount;

	// Impacts: new contacts and hard hits on existing ones, once per object
	// pair per step.
	struct Impact_t
	{
		CPhysicsObjectBox3D *pA;
		CPhysicsObjectBox3D *pB;
		Vector point;
		Vector normal;
	};
	CUtlVector<Impact_t> impacts;
	auto addImpact = [&]( CPhysicsObjectBox3D *pA, CPhysicsObjectBox3D *pB, const Vector &point, const Vector &normal )
	{
		if ( !IsLive( pA ) || !IsLive( pB ) )
			return;
		for ( int i = 0; i < impacts.Count(); i++ )
		{
			if ( ( impacts[i].pA == pA && impacts[i].pB == pB ) || ( impacts[i].pA == pB && impacts[i].pB == pA ) )
				return;
		}
		Impact_t impact = { pA, pB, point, normal };
		impacts.AddToTail( impact );
	};
	for ( int i = 0; i < events.beginCount; i++ )
	{
		const b3ContactBeginTouchEvent &begin = events.beginEvents[i];
		if ( !b3Contact_IsValid( begin.contactId ) )
			continue;
		b3ContactData data = b3Contact_GetData( begin.contactId );
		Vector point( 0, 0, 0 ), normal( 0, 0, 1 );
		ContactGeometry( data, &point, &normal );
		addImpact( ObjectOf( begin.shapeIdA ), ObjectOf( begin.shapeIdB ), point, normal );
	}
	for ( int i = 0; i < events.hitCount; i++ )
	{
		const b3ContactHitEvent &hit = events.hitEvents[i];
		addImpact( ObjectOf( hit.shapeIdA ), ObjectOf( hit.shapeIdB ), FromB3( hit.point ), FromB3( hit.normal ) );
	}

	for ( int i = 0; i < impacts.Count(); i++ )
	{
		CPhysicsObjectBox3D *pA = impacts[i].pA;
		CPhysicsObjectBox3D *pB = impacts[i].pB;
		if ( !IsLive( pA ) || !IsLive( pB ) )
			continue;
		// Objects that touched something dynamic wake their neighbors when
		// deleted (IVP's hasTouchedDynamic).
		if ( !pA->IsStatic() || !pB->IsStatic() )
		{
			if ( pB->IsMoveable() )
				pA->SetTouchedDynamic();
			if ( pA->IsMoveable() )
				pB->SetTouchedDynamic();
		}
		// Every impact counts, reported or not (IVP's impact statistics).
		{
			float closing = DotProduct( pA->GetPreStepVelocity() - pB->GetPreStepVelocity(), impacts[i].normal );
			float invMass = ( pA->IsMoveable() ? pA->GetInvMass() : 0.0f ) + ( pB->IsMoveable() ? pB->GetInvMass() : 0.0f );
			m_stats.impactCounter++;
			if ( pA->IsStatic() || pB->IsStatic() )
				m_stats.impactStaticCount++;
			// Kinetic energy of the closing motion (J), the most an impact
			// can destroy.
			if ( invMass > 0.0f )
				m_stats.totalEnergyDestroyed += 0.5 * closing * closing / invMass / kInertiaToBox3D;
		}
		if ( !m_pCollisionEvents )
			continue;

		// Gating matches IVP's CPhysicsListenerCollision::event_pre_collision.
		unsigned int flagsA = pA->GetCallbackFlags();
		unsigned int flagsB = pB->GetCallbackFlags();
		vcollisionevent_t event;
		memset( &event, 0, sizeof( event ) );
		event.isCollision = ( flagsA & flagsB & CALLBACK_GLOBAL_COLLISION ) != 0;
		event.isShadowCollision = ( ( flagsA ^ flagsB ) & CALLBACK_SHADOW_COLLISION ) != 0;
		if ( pA->IsStatic() && !( flagsB & CALLBACK_GLOBAL_COLLIDE_STATIC ) )
			event.isCollision = false;
		if ( pB->IsStatic() && !( flagsA & CALLBACK_GLOBAL_COLLIDE_STATIC ) )
			event.isCollision = false;
		if ( !event.isCollision && !event.isShadowCollision )
			continue;

		Vector relative = pA->GetPreStepVelocity() - pB->GetPreStepVelocity();

		event.pObjects[0] = pA;
		event.pObjects[1] = pB;
		event.surfaceProps[0] = pA->GetMaterialIndex();
		event.surfaceProps[1] = pB->GetMaterialIndex();
		event.deltaCollisionTime = PairDeltaTime( pA, pB );
		CCollisionDataBox3D collisionData( impacts[i].normal, impacts[i].point, relative );
		event.pInternalData = &collisionData;

		pA->ReportPreStepVelocity( true );
		pB->ReportPreStepVelocity( true );
		m_pCollisionEvents->PreCollision( &event );
		pA->ReportPreStepVelocity( false );
		pB->ReportPreStepVelocity( false );

		event.collisionSpeed = fabsf( DotProduct( relative, impacts[i].normal ) );
		m_pCollisionEvents->PostCollision( &event );
	}
}

// IVP's event_friction_created/deleted: touch events for objects in contact,
// when either wants touch callbacks, and for static partners only with
// CALLBACK_GLOBAL_TOUCH_STATIC.
void CPhysicsEnvironmentBox3D::DispatchTouchEvents()
{
	if ( !m_pCollisionEvents )
		return;
	b3ContactEvents events = b3World_GetContactEvents( m_world );
	for ( int pass = 0; pass < 2; pass++ )
	{
		int count = pass == 0 ? events.beginCount : events.endCount;
		for ( int i = 0; i < count; i++ )
		{
			b3ShapeId shapeA = pass == 0 ? events.beginEvents[i].shapeIdA : events.endEvents[i].shapeIdA;
			b3ShapeId shapeB = pass == 0 ? events.beginEvents[i].shapeIdB : events.endEvents[i].shapeIdB;
			CPhysicsObjectBox3D *pA = ObjectOf( shapeA );
			CPhysicsObjectBox3D *pB = ObjectOf( shapeB );
			if ( !IsLive( pA ) || !IsLive( pB ) || pA == pB )
				continue;
			unsigned int allFlags = pA->GetCallbackFlags() | pB->GetCallbackFlags();
			if ( !( allFlags & CALLBACK_GLOBAL_TOUCH ) )
				continue;
			if ( ( pA->IsStatic() || pB->IsStatic() ) && !( allFlags & CALLBACK_GLOBAL_TOUCH_STATIC ) )
				continue;
			Vector point( 0, 0, 0 ), normal( 0, 0, 1 );
			if ( pass == 0 && b3Contact_IsValid( events.beginEvents[i].contactId ) )
				ContactGeometry( b3Contact_GetData( events.beginEvents[i].contactId ), &point, &normal );
			CCollisionDataBox3D data( normal, point, vec3_origin );
			if ( pass == 0 )
				m_pCollisionEvents->StartTouch( pA, pB, &data );
			else
				m_pCollisionEvents->EndTouch( pA, pB, &data );
		}
	}
}

void CPhysicsEnvironmentBox3D::TriggerRemoved( CPhysicsObjectBox3D *pTrigger )
{
	for ( int i = m_triggerOverlaps.Count() - 1; i >= 0; i-- )
	{
		if ( m_triggerOverlaps[i].pTrigger == pTrigger )
			m_triggerOverlaps.FastRemove( i );
	}
}

void CPhysicsEnvironmentBox3D::WakeTriggerOverlaps( CPhysicsObjectBox3D *pTrigger )
{
	for ( int i = 0; i < m_triggerOverlaps.Count(); i++ )
	{
		if ( m_triggerOverlaps[i].pTrigger == pTrigger && IsLive( m_triggerOverlaps[i].pObject ) )
			m_triggerOverlaps[i].pObject->Wake();
	}
}

// IVP phantoms (triggers and fluids) report cores entering and leaving
// their volume, ignoring unmoveable ones. Box3D reports per shape pair, so
// overlaps are counted per trigger/object pair.
void CPhysicsEnvironmentBox3D::DispatchSensorEvents()
{
	b3SensorEvents events = b3World_GetSensorEvents( m_world );
	for ( int pass = 0; pass < 2; pass++ )
	{
		int count = pass == 0 ? events.beginCount : events.endCount;
		for ( int i = 0; i < count; i++ )
		{
			b3ShapeId sensor = pass == 0 ? events.beginEvents[i].sensorShapeId : events.endEvents[i].sensorShapeId;
			b3ShapeId visitor = pass == 0 ? events.beginEvents[i].visitorShapeId : events.endEvents[i].visitorShapeId;
			CPhysicsObjectBox3D *pTrigger = ObjectOf( sensor );
			CPhysicsObjectBox3D *pObject = ObjectOf( visitor );
			if ( !IsLive( pTrigger ) || !IsLive( pObject ) || !pTrigger->IsTrigger() || pObject->IsStatic() )
				continue;

			int index = m_triggerOverlaps.InvalidIndex();
			for ( int k = 0; k < m_triggerOverlaps.Count(); k++ )
			{
				if ( m_triggerOverlaps[k].pTrigger == pTrigger && m_triggerOverlaps[k].pObject == pObject )
				{
					index = k;
					break;
				}
			}
			bool entered = false, left = false;
			if ( pass == 0 )
			{
				if ( index == m_triggerOverlaps.InvalidIndex() )
				{
					TriggerOverlap_t overlap = { pTrigger, pObject, 0 };
					index = m_triggerOverlaps.AddToTail( overlap );
					entered = true;
				}
				m_triggerOverlaps[index].shapeCount++;
			}
			else
			{
				if ( index == m_triggerOverlaps.InvalidIndex() )
					continue;
				if ( --m_triggerOverlaps[index].shapeCount <= 0 )
				{
					m_triggerOverlaps.FastRemove( index );
					left = true;
				}
			}
			if ( !m_pCollisionEvents || ( !entered && !left ) )
				continue;
			CPhysicsFluidControllerBox3D *pFluid = pTrigger->GetFluidController();
			if ( pFluid )
			{
				if ( pObject->GetCallbackFlags() & CALLBACK_FLUID_TOUCH )
				{
					if ( entered )
						m_pCollisionEvents->FluidStartTouch( pObject, pFluid );
					else
						m_pCollisionEvents->FluidEndTouch( pObject, pFluid );
				}
			}
			else if ( entered )
			{
				m_pCollisionEvents->ObjectEnterTrigger( pTrigger, pObject );
			}
			else
			{
				m_pCollisionEvents->ObjectLeaveTrigger( pTrigger, pObject );
			}
		}
	}
}

// IVP's CSleepObjects::ProcessActiveObjects: sliding contacts report the
// energy their friction removed, per unit mass, when above a threshold.
void CPhysicsEnvironmentBox3D::DispatchFrictionEvents( float dt )
{
	if ( !m_pCollisionEvents || dt <= 0 )
		return;
	CUtlVector<IPhysicsObject *> objects;
	objects.CopyArray( m_objects.Base(), m_objects.Count() );
	CUtlVector<b3ContactData> contacts;
	for ( int i = 0; i < objects.Count(); i++ )
	{
		CPhysicsObjectBox3D *pObject = ToBox3D( objects[i] );
		if ( !IsLive( pObject ) || !pObject->IsMoveable() || pObject->IsAsleep() )
			continue;
		if ( !( pObject->GetCallbackFlags() & CALLBACK_GLOBAL_FRICTION ) )
			continue;
		b3BodyId body = pObject->GetBody();
		int capacity = b3Body_GetContactCapacity( body );
		if ( capacity <= 0 )
			continue;
		contacts.SetCount( capacity );
		int count = b3Body_GetContactData( body, contacts.Base(), capacity );
		for ( int c = 0; c < count; c++ )
		{
			const b3ContactData &data = contacts[c];
			CPhysicsObjectBox3D *pA = ObjectOf( data.shapeIdA );
			CPhysicsObjectBox3D *pB = ObjectOf( data.shapeIdB );
			CPhysicsObjectBox3D *pOther = pA == pObject ? pB : pA;
			if ( !IsLive( pOther ) || pOther == pObject || !( pOther->GetCallbackFlags() & CALLBACK_GLOBAL_FRICTION ) )
				continue;
			// Each contact is reported once: by the lower object when both slide.
			if ( pOther->IsMoveable() && !pOther->IsAsleep() && pOther < pObject )
				continue;
			float friction = MixFriction( pObject->GetFriction(), 0, pOther->GetFriction(), 0 );
			float energy = 0.0f;
			for ( int m = 0; m < data.manifoldCount; m++ )
			{
				const b3Manifold &manifold = data.manifolds[m];
				Vector normal = FromB3( manifold.normal );
				Vector centerA = FromB3( b3Body_GetWorldCenter( b3Shape_GetBody( data.shapeIdA ) ) );
				for ( int p = 0; p < manifold.pointCount; p++ )
				{
					Vector point = centerA + FromB3( manifold.points[p].anchorA );
					Vector velocityA, velocityB;
					pA->GetVelocityAtPoint( point, &velocityA );
					pB->GetVelocityAtPoint( point, &velocityB );
					Vector slip = velocityA - velocityB;
					slip -= normal * DotProduct( slip, normal );
					// Work done by kinetic friction over the step (Source units).
					energy += friction * manifold.points[p].totalNormalImpulse * slip.Length();
				}
			}
			// Havok units, per second, per unit mass (IVP's scrape measure).
			float scaled = energy / kInertiaToBox3D / dt * pObject->GetInvMass();
			if ( scaled <= 0.05f )
				continue;
			Vector point( 0, 0, 0 ), normal( 0, 0, 1 );
			ContactGeometry( data, &point, &normal );
			if ( pA != pObject )
				normal = -normal;
			CCollisionDataBox3D collisionData( normal, point, vec3_origin );
			m_stats.frictionEventsProcessed++;
			m_pCollisionEvents->Friction( pObject, scaled * kInertiaToBox3D, pObject->GetMaterialIndex(),
				pOther->GetMaterialIndex(), &collisionData );
		}
	}
}

void CPhysicsEnvironmentBox3D::DispatchSleepWakeEvents()
{
	// Callbacks may destroy objects; walk a snapshot and re-check membership.
	CUtlVector<IPhysicsObject *> objects;
	objects.CopyArray( m_objects.Base(), m_objects.Count() );
	for ( int i = 0; i < objects.Count(); i++ )
	{
		if ( !IsLive( objects[i] ) )
			continue;
		CPhysicsObjectBox3D *pObject = ToBox3D( objects[i] );
		if ( pObject->IsStatic() )
			continue;
		bool awake = !pObject->IsAsleep();
		if ( awake == pObject->WasAwake() )
			continue;
		pObject->SetWasAwake( awake );
		if ( awake )
			pObject->SetAsleepSinceCreation( false );
		if ( !m_pObjectEvents )
			continue;
		if ( awake )
			m_pObjectEvents->ObjectWake( pObject );
		else
			m_pObjectEvents->ObjectSleep( pObject );
	}
}

void CPhysicsEnvironmentBox3D::PostStep( float dt )
{
	CheckConstraintBreaks( dt );
	DispatchContactEvents( dt );
	DispatchTouchEvents();
	DispatchSensorEvents();
	DispatchFrictionEvents( dt );
	DispatchSleepWakeEvents();
}

void CPhysicsEnvironmentBox3D::Step( float dt )
{
	// Wakes from game calls made between steps are reported too.
	double start = Plat_FloatTime();
	DispatchSleepWakeEvents();
	PreStep( dt );
	double solverStart = Plat_FloatTime();
	b3World_Step( m_world, dt, kSubSteps );
	double solverEnd = Plat_FloatTime();
	m_stepCount++;
	m_simulationTime += dt;
	PostStep( dt );
	m_preStepSeconds += solverStart - start;
	m_stepSeconds += solverEnd - solverStart;
	m_postStepSeconds += Plat_FloatTime() - solverEnd;
}

void CPhysicsEnvironmentBox3D::Simulate( float deltaTime )
{
	double simulateStart = Plat_FloatTime();
	int stepsBefore = m_stepCount;
	m_simulateThread = ThreadGetCurrentId();
	m_stepSeconds = m_preStepSeconds = m_postStepSeconds = 0.0;
	ClearDeadObjects();
	// As IVP: ignore clock jumps and sub-0.1 ms calls; cap at 100 ms.
	if ( deltaTime <= 1.0f && deltaTime > 0.0001f )
	{
		if ( deltaTime > 0.1f )
			deltaTime = 0.1f;
		// IVP reports the previous frame's end at the start of each call.
		if ( m_pCollisionEvents )
			m_pCollisionEvents->PostSimulationFrame();

		m_inSimulation = true;
		m_timeAccumulator += deltaTime;
		int steps = 0;
		while ( m_timeAccumulator >= m_timestep * 0.999f && steps < kMaxStepsPerSimulate )
		{
			Step( m_timestep );
			m_timeAccumulator -= m_timestep;
			steps++;
		}
		if ( m_timeAccumulator < 0.0f || steps == kMaxStepsPerSimulate )
			m_timeAccumulator = 0.0f;
		m_inSimulation = false;
	}
	// Without a queue, deletions requested during the step happen now.
	if ( !m_queueDeleteObject )
		ClearDeadObjects();

	b3Counters counters = b3World_GetCounters( m_world );
	m_lastProfile.stepCount = m_stepCount - stepsBefore;
	m_lastProfile.stepMs = (float)( m_stepSeconds * 1000.0 );
	m_lastProfile.preStepMs = (float)( m_preStepSeconds * 1000.0 );
	m_lastProfile.postStepMs = (float)( m_postStepSeconds * 1000.0 );
	m_lastProfile.bodyCount = counters.bodyCount;
	m_lastProfile.awakeBodyCount = b3World_GetAwakeBodyCount( m_world );
	m_lastProfile.contactCount = counters.contactCount;
	m_lastProfile.workerCount = m_workerCount;
	m_lastProfile.solverCalls = m_solverCalls;
	m_lastProfile.solverCallsOffCaller = m_solverCallsOffCaller;
	m_lastProfile.simulateMs = (float)( ( Plat_FloatTime() - simulateStart ) * 1000.0 );
}

bool CPhysicsEnvironmentBox3D::IsInSimulation() const { return m_inSimulation; }

float CPhysicsEnvironmentBox3D::GetSimulationTime() const { return m_simulationTime; }
float CPhysicsEnvironmentBox3D::GetNextFrameTime() const { return m_simulationTime + m_timestep - m_timeAccumulator; }

void CPhysicsEnvironmentBox3D::SetSimulationTimestep( float timestep )
{
	if ( timestep > 0.0f )
		m_timestep = timestep;
}

void CPhysicsEnvironmentBox3D::ResetSimulationClock()
{
	m_simulationTime = 0.0f;
	m_timeAccumulator = 0.0f;
	m_impactPairs.RemoveAll();
}

void CPhysicsEnvironmentBox3D::SetCollisionEventHandler( IPhysicsCollisionEvent *pCollisionEvents ) { m_pCollisionEvents = pCollisionEvents; }
void CPhysicsEnvironmentBox3D::SetObjectEventHandler( IPhysicsObjectEvent *pObjectEvents ) { m_pObjectEvents = pObjectEvents; }
void CPhysicsEnvironmentBox3D::SetConstraintEventHandler( IPhysicsConstraintEvent *pConstraintEvents ) { m_pConstraintEvents = pConstraintEvents; }

void CPhysicsEnvironmentBox3D::SetQuickDelete( bool bQuick ) { m_quickDelete = bQuick; }

int CPhysicsEnvironmentBox3D::GetActiveObjectCount() const
{
	int count = 0;
	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		const CPhysicsObjectBox3D *pObject = static_cast<const CPhysicsObjectBox3D *>( m_objects[i] );
		if ( !pObject->IsStatic() && !pObject->IsAsleep() )
			count++;
	}
	return count;
}

void CPhysicsEnvironmentBox3D::GetActiveObjects( IPhysicsObject **pOutputObjectList ) const
{
	int out = 0;
	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		const CPhysicsObjectBox3D *pObject = static_cast<const CPhysicsObjectBox3D *>( m_objects[i] );
		if ( !pObject->IsStatic() && !pObject->IsAsleep() )
			pOutputObjectList[out++] = m_objects[i];
	}
}

const IPhysicsObject **CPhysicsEnvironmentBox3D::GetObjectList( int *pOutputObjectCount ) const
{
	if ( pOutputObjectCount )
		*pOutputObjectCount = m_objects.Count();
	return m_objects.Count() ? const_cast<const IPhysicsObject **>( m_objects.Base() ) : NULL;
}


// IVP leaves these environment queries unimplemented; game code traces
// through the engine and IPhysicsCollision instead.
void CPhysicsEnvironmentBox3D::TraceRay( const Ray_t &ray, unsigned int fMask, IPhysicsTraceFilter *pTraceFilter, trace_t *pTrace ) {}
void CPhysicsEnvironmentBox3D::SweepCollideable( const CPhysCollide *pCollide, const Vector &vecAbsStart, const Vector &vecAbsEnd, const QAngle &vecAngles, unsigned int fMask, IPhysicsTraceFilter *pTraceFilter, trace_t *pTrace ) {}

void CPhysicsEnvironmentBox3D::GetPerformanceSettings( physics_performanceparams_t *pOutput ) const
{
	if ( pOutput )
		*pOutput = m_performance;
}

void CPhysicsEnvironmentBox3D::SetPerformanceSettings( const physics_performanceparams_t *pSettings )
{
	if ( !pSettings )
		return;
	m_performance = *pSettings;
	m_performance.minFrictionMass = clamp( m_performance.minFrictionMass, 1.0f, VPHYSICS_MAX_MASS );
	m_performance.maxFrictionMass = clamp( m_performance.maxFrictionMass, 1.0f, VPHYSICS_MAX_MASS );
	b3World_SetMaximumLinearSpeed( m_world, m_performance.maxVelocity );
}

// IVP reports its statistic manager's counters; the Box3D equivalents are
// kept since the last ClearStats: impacts (and those against static
// objects), contact pairs begun and ended, the pairs currently touching, and
// friction events. IVP's rescue/speed-gain and range counters have no Box3D
// counterpart and read zero.
void CPhysicsEnvironmentBox3D::ReadStats( physics_stats_t *pOutput )
{
	if ( !pOutput )
		return;
	*pOutput = m_stats;
	b3Counters counters = b3World_GetCounters( m_world );
	pOutput->collisionPairsTotal = counters.contactCount;
}

void CPhysicsEnvironmentBox3D::ClearStats()
{
	memset( &m_stats, 0, sizeof( m_stats ) );
}

void CPhysicsEnvironmentBox3D::DebugCheckContacts( void ) {}
