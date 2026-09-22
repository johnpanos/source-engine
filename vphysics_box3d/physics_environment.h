#ifndef PHYSICS_ENVIRONMENT_H
#define PHYSICS_ENVIRONMENT_H

#include "vphysics_interface.h"
#include "vphysics/performance.h"
#include "utlvector.h"
#include "box3d/id.h"
#include "physics_controllers.h"

class CPhysicsObjectBox3D;

// One VPhysics environment is one Box3D world, stepped on the caller's thread
// with a single worker (RFC 0004 B: one-worker vertical slice). Simulate()
// advances in fixed steps of the simulation timestep, as IVP does; each step
// runs the controllers (PreStep), the Box3D step, then dispatches contact,
// sleep and wake events to the game (PostStep) outside the Box3D step, so
// game callbacks may create or destroy objects.
class CPhysicsEnvironmentBox3D : public IPhysicsEnvironment
{
public:
	CPhysicsEnvironmentBox3D();
	virtual ~CPhysicsEnvironmentBox3D();

	virtual void SetDebugOverlay( CreateInterfaceFn debugOverlayFactory ) override;
	virtual IVPhysicsDebugOverlay *GetDebugOverlay( void ) override;

	virtual void SetGravity( const Vector &gravityVector ) override;
	virtual void GetGravity( Vector *pGravityVector ) const override;

	virtual void SetAirDensity( float density ) override;
	virtual float GetAirDensity( void ) const override;

	virtual IPhysicsObject *CreatePolyObject( const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, struct objectparams_t *pParams ) override;
	virtual IPhysicsObject *CreatePolyObjectStatic( const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, struct objectparams_t *pParams ) override;
	virtual IPhysicsObject *CreateSphereObject( float radius, int materialIndex, const Vector &position, const QAngle &angles, struct objectparams_t *pParams, bool isStatic ) override;
	virtual void DestroyObject( IPhysicsObject *pObject ) override;

	virtual IPhysicsFluidController *CreateFluidController( IPhysicsObject *pFluidObject, fluidparams_t *pParams ) override;
	virtual void DestroyFluidController( IPhysicsFluidController * ) override;

	virtual IPhysicsSpring *CreateSpring( IPhysicsObject *pObjectStart, IPhysicsObject *pObjectEnd, springparams_t *pParams ) override;
	virtual void DestroySpring( IPhysicsSpring * ) override;

	virtual IPhysicsConstraint *CreateRagdollConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_ragdollparams_t &ragdoll ) override;
	virtual IPhysicsConstraint *CreateHingeConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_hingeparams_t &hinge ) override;
	virtual IPhysicsConstraint *CreateFixedConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_fixedparams_t &fixed ) override;
	virtual IPhysicsConstraint *CreateSlidingConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_slidingparams_t &sliding ) override;
	virtual IPhysicsConstraint *CreateBallsocketConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_ballsocketparams_t &ballsocket ) override;
	virtual IPhysicsConstraint *CreatePulleyConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_pulleyparams_t &pulley ) override;
	virtual IPhysicsConstraint *CreateLengthConstraint( IPhysicsObject *pReferenceObject, IPhysicsObject *pAttachedObject, IPhysicsConstraintGroup *pGroup, const constraint_lengthparams_t &length ) override;
	virtual void DestroyConstraint( IPhysicsConstraint * ) override;

	virtual IPhysicsConstraintGroup *CreateConstraintGroup( const constraint_groupparams_t &pParams ) override;
	virtual void DestroyConstraintGroup( IPhysicsConstraintGroup *pGroup ) override;

	virtual IPhysicsShadowController *CreateShadowController( IPhysicsObject *pObject, bool allowTranslation, bool allowRotation ) override;
	virtual void DestroyShadowController( IPhysicsShadowController * ) override;

	virtual IPhysicsPlayerController *CreatePlayerController( IPhysicsObject *pObject ) override;
	virtual void DestroyPlayerController( IPhysicsPlayerController * ) override;

	virtual IPhysicsMotionController *CreateMotionController( IMotionEvent *pHandler ) override;
	virtual void DestroyMotionController( IPhysicsMotionController *pController ) override;

	virtual IPhysicsVehicleController *CreateVehicleController( IPhysicsObject *pVehicleBodyObject, const vehicleparams_t &params, unsigned int nVehicleType, IPhysicsGameTrace *pGameTrace ) override;
	virtual void DestroyVehicleController( IPhysicsVehicleController * ) override;

	virtual void SetCollisionSolver( IPhysicsCollisionSolver *pSolver ) override;
	virtual void Simulate( float deltaTime ) override;
	virtual bool IsInSimulation() const override;

	virtual float GetSimulationTime() const override;
	virtual float GetNextFrameTime() const override;
	virtual float GetSimulationTimestep() const override { return m_timestep; }
	virtual void SetSimulationTimestep( float timestep ) override;
	virtual void ResetSimulationClock() override;

	virtual void SetCollisionEventHandler( IPhysicsCollisionEvent *pCollisionEvents ) override;
	virtual void SetObjectEventHandler( IPhysicsObjectEvent *pObjectEvents ) override;
	virtual void SetConstraintEventHandler( IPhysicsConstraintEvent *pConstraintEvents ) override;

	virtual void SetQuickDelete( bool bQuick ) override;

	virtual int GetActiveObjectCount() const override;
	virtual void GetActiveObjects( IPhysicsObject **pOutputObjectList ) const override;
	virtual const IPhysicsObject **GetObjectList( int *pOutputObjectCount ) const override;

	virtual bool TransferObject( IPhysicsObject *pObject, IPhysicsEnvironment *pDestinationEnvironment ) override;

	virtual void CleanupDeleteList( void ) override;
	virtual void EnableDeleteQueue( bool enable ) override;

	virtual bool Save( const physsaveparams_t &params ) override;
	virtual void PreRestore( const physprerestoreparams_t &params ) override;
	virtual bool Restore( const physrestoreparams_t &params ) override;
	virtual void PostRestore() override;

	virtual bool IsCollisionModelUsed( CPhysCollide *pCollide ) const override;
	virtual void TraceRay( const Ray_t &ray, unsigned int fMask, IPhysicsTraceFilter *pTraceFilter, trace_t *pTrace ) override;
	virtual void SweepCollideable( const CPhysCollide *pCollide, const Vector &vecAbsStart, const Vector &vecAbsEnd, const QAngle &vecAngles, unsigned int fMask, IPhysicsTraceFilter *pTraceFilter, trace_t *pTrace ) override;

	virtual void GetPerformanceSettings( physics_performanceparams_t *pOutput ) const override;
	virtual void SetPerformanceSettings( const physics_performanceparams_t *pSettings ) override;

	virtual void ReadStats( physics_stats_t *pOutput ) override;
	virtual void ClearStats() override;

	virtual unsigned int GetObjectSerializeSize( IPhysicsObject *pObject ) const override;
	virtual void SerializeObjectToBuffer( IPhysicsObject *pObject, unsigned char *pBuffer, unsigned int bufferSize ) override;
	virtual IPhysicsObject *UnserializeObjectFromBuffer( void *pGameData, unsigned char *pBuffer, unsigned int bufferSize, bool enableCollisions ) override;

	virtual void EnableConstraintNotify( bool bEnable ) override;
	virtual void DebugCheckContacts( void ) override;

	// Provider internals.
	b3WorldId GetWorld() const { return m_world; }
	float GetStepTime() const { return m_timestep; }

private:
	static bool CustomFilter( b3ShapeId shapeIdA, b3ShapeId shapeIdB, void *pContext );
	IPhysicsObject *TrackObject( CPhysicsObjectBox3D *pObject );
	void Step( float dt );
	void PreStep( float dt );
	void PostStep();
	void DispatchContactEvents();
	void DispatchSleepWakeEvents();

	b3WorldId m_world;
	Vector m_gravity;
	float m_airDensity;
	float m_timestep;
	float m_timeAccumulator;
	float m_simulationTime;
	bool m_inSimulation;
	physics_performanceparams_t m_performance;
	IPhysicsCollisionSolver *m_pSolver;
	IPhysicsCollisionEvent *m_pCollisionEvents;
	IPhysicsObjectEvent *m_pObjectEvents;
	IPhysicsConstraintEvent *m_pConstraintEvents;
	CUtlVector<IPhysicsObject *> m_objects;
	CUtlVector<CMotionControllerBox3D *> m_motionControllers;
	CUtlVector<CPlayerControllerBox3D *> m_playerControllers;
	CUtlVector<CConstraintBox3D *> m_constraints;
};

#endif // PHYSICS_ENVIRONMENT_H
