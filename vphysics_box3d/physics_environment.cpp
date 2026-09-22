//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics environment backed by a Box3D world (RFC 0004 B2/B5).
//
//=============================================================================//
#include "physics_environment.h"

#include <string.h>

#include "box3d/box3d.h"
#include "box3d_convert.h"
#include "cmodel.h"
#include "gametrace.h"
#include "physics_collision.h"
#include "physics_constraint.h"
#include "physics_object.h"
#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const float kDefaultTimestep = 0.015f;	// IVP's default PSI (DEFAULT_TICK_INTERVAL)
const int kSubSteps = 4;

CPhysicsObjectBox3D *ObjectOf( b3ShapeId shape )
{
	return b3Shape_IsValid( shape ) ? (CPhysicsObjectBox3D *)b3Shape_GetUserData( shape ) : NULL;
}

// Contact geometry handed to PreCollision/PostCollision as pInternalData.
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
}

CPhysicsEnvironmentBox3D::CPhysicsEnvironmentBox3D()
	: m_airDensity( 2.0f ), m_timestep( kDefaultTimestep ), m_timeAccumulator( 0.0f ), m_simulationTime( 0.0f ), m_stepCount( 0 ),
	  m_inSimulation( false ), m_pSolver( NULL ), m_pCollisionEvents( NULL ), m_pObjectEvents( NULL ),
	  m_pConstraintEvents( NULL ), m_quickDelete( false ), m_enableConstraintNotify( false )
{
	m_gravity.Init();
	m_performance.Defaults();

	b3WorldDef def = b3DefaultWorldDef();
	def.gravity = b3Vec3_zero;
	def.enableSleep = true;
	def.enableContinuous = true;
	def.workerCount = 1;
	def.maximumLinearSpeed = m_performance.maxVelocity;
	def.userData = this;
	m_world = b3CreateWorld( &def );
	b3World_SetCustomFilterCallback( m_world, CustomFilter, this );
}

CPhysicsEnvironmentBox3D::~CPhysicsEnvironmentBox3D()
{
	// Game code normally destroys its objects first; release anything left.
	m_constraints.PurgeAndDeleteElements();
	m_constraintGroups.PurgeAndDeleteElements();
	m_motionControllers.PurgeAndDeleteElements();
	m_playerControllers.PurgeAndDeleteElements();
	for ( int i = m_objects.Count() - 1; i >= 0; i-- )
		delete m_objects[i];
	m_objects.RemoveAll();
	b3DestroyWorld( m_world );
}

void CPhysicsEnvironmentBox3D::SetDebugOverlay( CreateInterfaceFn debugOverlayFactory ) {}
IVPhysicsDebugOverlay *CPhysicsEnvironmentBox3D::GetDebugOverlay( void ) { return nullptr; }

void CPhysicsEnvironmentBox3D::SetGravity( const Vector &gravityVector )
{
	m_gravity = gravityVector;
	b3World_SetGravity( m_world, ToB3( gravityVector ) );
}

void CPhysicsEnvironmentBox3D::GetGravity( Vector *pGravityVector ) const { *pGravityVector = m_gravity; }

void CPhysicsEnvironmentBox3D::SetAirDensity( float density ) { m_airDensity = density; }
float CPhysicsEnvironmentBox3D::GetAirDensity( void ) const { return m_airDensity; }

//-----------------------------------------------------------------------------
// Objects
//-----------------------------------------------------------------------------
IPhysicsObject *CPhysicsEnvironmentBox3D::TrackObject( CPhysicsObjectBox3D *pObject )
{
	m_objects.AddToTail( pObject );
	return pObject;
}

IPhysicsObject *CPhysicsEnvironmentBox3D::CreatePolyObject( const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, objectparams_t *pParams )
{
	return TrackObject( new CPhysicsObjectBox3D( this, pCollisionModel, 0.0f, materialIndex, position, angles, pParams, false ) );
}

IPhysicsObject *CPhysicsEnvironmentBox3D::CreatePolyObjectStatic( const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, objectparams_t *pParams )
{
	return TrackObject( new CPhysicsObjectBox3D( this, pCollisionModel, 0.0f, materialIndex, position, angles, pParams, true ) );
}

IPhysicsObject *CPhysicsEnvironmentBox3D::CreateSphereObject( float radius, int materialIndex, const Vector &position, const QAngle &angles, objectparams_t *pParams, bool isStatic )
{
	return TrackObject( new CPhysicsObjectBox3D( this, NULL, radius, materialIndex, position, angles, pParams, isStatic ) );
}

void CPhysicsEnvironmentBox3D::DestroyObject( IPhysicsObject *pObject )
{
	if ( !pObject )
		return;
	m_objects.FindAndRemove( pObject );
	// Box3D destroys a body's joints with it; its constraints become inert,
	// and (when enabled) the game hears about each as IVP reports it.
	for ( int i = 0; i < m_constraints.Count(); i++ )
	{
		CConstraintBox3D *pConstraint = m_constraints[i];
		if ( !pConstraint->Links( pObject ) )
			continue;
		pConstraint->ObjectDestroyed();
		if ( m_enableConstraintNotify && m_pConstraintEvents )
			m_pConstraintEvents->ConstraintBroken( pConstraint );
	}
	for ( int i = 0; i < m_motionControllers.Count(); i++ )
		m_motionControllers[i]->ObjectDestroyed( pObject );
	delete ToBox3D( pObject );
}

IPhysicsFluidController *CPhysicsEnvironmentBox3D::CreateFluidController( IPhysicsObject *pFluidObject, fluidparams_t *pParams ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroyFluidController( IPhysicsFluidController * ) {}

IPhysicsSpring *CPhysicsEnvironmentBox3D::CreateSpring( IPhysicsObject *pObjectStart, IPhysicsObject *pObjectEnd, springparams_t *pParams ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroySpring( IPhysicsSpring * ) {}

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
	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		CPhysicsObjectBox3D *pObject = ToBox3D( m_objects[i] );
		if ( pObject->GetShadowController() == pController )
		{
			pObject->RemoveShadowController();
			return;
		}
	}
}

IPhysicsPlayerController *CPhysicsEnvironmentBox3D::CreatePlayerController( IPhysicsObject *pObject )
{
	CPlayerControllerBox3D *pController = new CPlayerControllerBox3D( ToBox3D( pObject ) );
	m_playerControllers.AddToTail( pController );
	return pController;
}

void CPhysicsEnvironmentBox3D::DestroyPlayerController( IPhysicsPlayerController *pController )
{
	CPlayerControllerBox3D *pBox = static_cast<CPlayerControllerBox3D *>( pController );
	if ( !m_playerControllers.FindAndRemove( pBox ) )
		return;
	CPhysicsObjectBox3D *pObject = ToBox3D( pBox->GetObject() );
	if ( pObject && pObject->GetPlayerController() == pBox )
		pObject->SetPlayerController( NULL );
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
	m_motionControllers.FindAndRemove( pBox );
	delete pBox;
}

IPhysicsVehicleController *CPhysicsEnvironmentBox3D::CreateVehicleController( IPhysicsObject *pVehicleBodyObject, const vehicleparams_t &params, unsigned int nVehicleType, IPhysicsGameTrace *pGameTrace ) { return nullptr; }
void CPhysicsEnvironmentBox3D::DestroyVehicleController( IPhysicsVehicleController * ) {}

//-----------------------------------------------------------------------------
// Simulation
//-----------------------------------------------------------------------------
bool CPhysicsEnvironmentBox3D::CustomFilter( b3ShapeId shapeIdA, b3ShapeId shapeIdB, void *pContext )
{
	CPhysicsEnvironmentBox3D *pEnv = static_cast<CPhysicsEnvironmentBox3D *>( pContext );
	CPhysicsObjectBox3D *pA = ObjectOf( shapeIdA );
	CPhysicsObjectBox3D *pB = ObjectOf( shapeIdB );
	if ( !pA || !pB || pA == pB )
		return false;
	if ( !pA->IsCollisionEnabled() || !pB->IsCollisionEnabled() )
		return false;
	// The game's collision rules (portal environments, player/prop filters).
	if ( pEnv->m_pSolver && !pEnv->m_pSolver->ShouldCollide( pA, pB, pA->GetGameData(), pB->GetGameData() ) )
		return false;
	return true;
}

void CPhysicsEnvironmentBox3D::SetCollisionSolver( IPhysicsCollisionSolver *pSolver ) { m_pSolver = pSolver; }

void CPhysicsEnvironmentBox3D::PreStep( float dt )
{
	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		CPhysicsObjectBox3D *pObject = ToBox3D( m_objects[i] );
		CShadowControllerBox3D *pShadow = pObject->GetShadow();
		if ( pShadow )
		{
			pObject->ApplyGravityScale( pShadow->TempDisablesGravity() );
			pShadow->Simulate( dt );
		}
	}
	for ( int i = 0; i < m_playerControllers.Count(); i++ )
		m_playerControllers[i]->Simulate( dt );
	for ( int i = 0; i < m_motionControllers.Count(); i++ )
		m_motionControllers[i]->Simulate( dt );
	for ( int i = 0; i < m_constraints.Count(); i++ )
		m_constraints[i]->PreStep( dt );

	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		CPhysicsObjectBox3D *pObject = ToBox3D( m_objects[i] );
		pObject->CapturePreStepVelocity();
		pObject->SetWasAwake( !pObject->IsAsleep() );
	}
}

void CPhysicsEnvironmentBox3D::DispatchContactEvents()
{
	if ( !m_pCollisionEvents )
		return;
	b3ContactEvents events = b3World_GetContactEvents( m_world );
	for ( int i = 0; i < events.beginCount; i++ )
	{
		const b3ContactBeginTouchEvent &begin = events.beginEvents[i];
		CPhysicsObjectBox3D *pA = ObjectOf( begin.shapeIdA );
		CPhysicsObjectBox3D *pB = ObjectOf( begin.shapeIdB );
		if ( !pA || !pB || !b3Contact_IsValid( begin.contactId ) )
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

		b3ContactData data = b3Contact_GetData( begin.contactId );
		Vector normal( 0, 0, 1 ), point( 0, 0, 0 );
		if ( data.manifoldCount > 0 && data.manifolds[0].pointCount > 0 )
		{
			const b3Manifold &manifold = data.manifolds[0];
			normal = FromB3( manifold.normal );
			Vector centerA = FromB3( b3Body_GetWorldCenter( pA->GetBody() ) );
			for ( int p = 0; p < manifold.pointCount; p++ )
				point += centerA + FromB3( manifold.points[p].anchorA );
			point /= (float)manifold.pointCount;
		}
		Vector relative = pA->GetPreStepVelocity() - pB->GetPreStepVelocity();

		event.pObjects[0] = pA;
		event.pObjects[1] = pB;
		event.surfaceProps[0] = pA->GetMaterialIndex();
		event.surfaceProps[1] = pB->GetMaterialIndex();
		event.deltaCollisionTime = 1.0f;	// IVP's value for a first/reset contact
		CCollisionDataBox3D collisionData( normal, point, relative );
		event.pInternalData = &collisionData;

		pA->ReportPreStepVelocity( true );
		pB->ReportPreStepVelocity( true );
		m_pCollisionEvents->PreCollision( &event );
		pA->ReportPreStepVelocity( false );
		pB->ReportPreStepVelocity( false );

		event.collisionSpeed = fabsf( DotProduct( relative, normal ) );
		m_pCollisionEvents->PostCollision( &event );
	}
}

void CPhysicsEnvironmentBox3D::DispatchSleepWakeEvents()
{
	if ( !m_pObjectEvents )
		return;
	// Callbacks may destroy objects; walk a snapshot and re-check membership.
	CUtlVector<IPhysicsObject *> objects;
	objects.CopyArray( m_objects.Base(), m_objects.Count() );
	for ( int i = 0; i < objects.Count(); i++ )
	{
		if ( m_objects.Find( objects[i] ) == m_objects.InvalidIndex() )
			continue;
		CPhysicsObjectBox3D *pObject = ToBox3D( objects[i] );
		if ( pObject->IsStatic() )
			continue;
		bool awake = !pObject->IsAsleep();
		if ( awake != pObject->WasAwake() )
		{
			if ( awake )
				m_pObjectEvents->ObjectWake( pObject );
			else
				m_pObjectEvents->ObjectSleep( pObject );
		}
	}
}

void CPhysicsEnvironmentBox3D::PostStep( float dt )
{
	CheckConstraintBreaks( dt );
	DispatchContactEvents();
	DispatchSleepWakeEvents();
}

void CPhysicsEnvironmentBox3D::Step( float dt )
{
	PreStep( dt );
	m_inSimulation = true;
	b3World_Step( m_world, dt, kSubSteps );
	m_stepCount++;
	m_inSimulation = false;
	PostStep( dt );
}

void CPhysicsEnvironmentBox3D::Simulate( float deltaTime )
{
	if ( deltaTime <= 0.0f )
		return;
	// Fixed steps of the simulation timestep, carrying the remainder, as IVP
	// advances its PSI clock.
	m_timeAccumulator += deltaTime;
	int steps = 0;
	while ( m_timeAccumulator >= m_timestep * 0.999f && steps < 8 )
	{
		Step( m_timestep );
		m_timeAccumulator -= m_timestep;
		steps++;
	}
	if ( m_timeAccumulator < 0.0f || steps == 8 )
		m_timeAccumulator = 0.0f;
	m_simulationTime += deltaTime;
	if ( m_pCollisionEvents )
		m_pCollisionEvents->PostSimulationFrame();
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

bool CPhysicsEnvironmentBox3D::TransferObject( IPhysicsObject *pObject, IPhysicsEnvironment *pDestinationEnvironment ) { return false; }

void CPhysicsEnvironmentBox3D::CleanupDeleteList( void ) {}
void CPhysicsEnvironmentBox3D::EnableDeleteQueue( bool enable ) {}

bool CPhysicsEnvironmentBox3D::Save( const physsaveparams_t &params ) { return false; }
void CPhysicsEnvironmentBox3D::PreRestore( const physprerestoreparams_t &params ) {}
bool CPhysicsEnvironmentBox3D::Restore( const physrestoreparams_t &params ) { return false; }
void CPhysicsEnvironmentBox3D::PostRestore() {}

bool CPhysicsEnvironmentBox3D::IsCollisionModelUsed( CPhysCollide *pCollide ) const
{
	for ( int i = 0; i < m_objects.Count(); i++ )
	{
		if ( m_objects[i]->GetCollide() == pCollide )
			return true;
	}
	return false;
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
	b3World_SetMaximumLinearSpeed( m_world, m_performance.maxVelocity );
}

void CPhysicsEnvironmentBox3D::ReadStats( physics_stats_t *pOutput ) {}
void CPhysicsEnvironmentBox3D::ClearStats() {}

unsigned int CPhysicsEnvironmentBox3D::GetObjectSerializeSize( IPhysicsObject *pObject ) const { return 0; }
void CPhysicsEnvironmentBox3D::SerializeObjectToBuffer( IPhysicsObject *pObject, unsigned char *pBuffer, unsigned int bufferSize ) {}
IPhysicsObject *CPhysicsEnvironmentBox3D::UnserializeObjectFromBuffer( void *pGameData, unsigned char *pBuffer, unsigned int bufferSize, bool enableCollisions ) { return nullptr; }

void CPhysicsEnvironmentBox3D::DebugCheckContacts( void ) {}
