#ifndef PHYSICS_ENVIRONMENT_H
#define PHYSICS_ENVIRONMENT_H

#include <mutex>

#include "tier0/threadtools.h"
#include "vphysics_interface.h"
#include "vphysics/performance.h"
#include "vphysics/step_profile.h"
#include "vphysics/stats.h"
#include "vphysics/constraints.h"
#include "utlvector.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "physics_controllers.h"

class CVehicleControllerBox3D;
class IThreadPool;

class CPhysicsObjectBox3D;
class CConstraintBox3D;
class CConstraintGroupBox3D;
class CPhysicsFluidControllerBox3D;
class CPhysicsSpringBox3D;

// One VPhysics environment is one Box3D world, stepped on the caller's thread.
// It uses one worker (RFC 0004 B: one-worker vertical slice) unless it was
// created through the opt-in parallel-step capability (RFC 0013), which fixes
// a larger worker count for the world's lifetime and runs Box3D's worker tasks
// on the caller's thread pool. Simulate()
// advances in fixed steps of the simulation timestep, as IVP does; each step
// runs the controllers, springs, pulleys, fluids, damping and drag
// (PreStep), the Box3D step, then dispatches constraint-break, contact,
// touch, trigger, fluid, friction, sleep and wake events to the game
// (PostStep) outside the Box3D step. The environment reports itself in
// simulation for the whole step (as IVP runs these callbacks inside its
// step), so objects destroyed from callbacks are queued and deleted once the
// step completes.
class CPhysicsEnvironmentBox3D : public IPhysicsEnvironment
{
public:
	// workerCount > 1 steps the world with that many workers, running Box3D's
	// worker tasks on pThreadPool (RFC 0013 vphysics.parallel-step.v1). The
	// pool must outlive the environment.
	explicit CPhysicsEnvironmentBox3D( int workerCount = 1, IThreadPool *pThreadPool = NULL );
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
	int GetWorkerCount() const { return m_workerCount; }
	// RFC 0013 vphysics.step-profile.v1: the last Simulate call.
	const physics_stepprofile_t &GetLastSimulateProfile() const { return m_lastProfile; }
	float GetStepTime() const { return m_timestep; }
	int GetStepCount() const { return m_stepCount; }
	bool ShouldQuickDelete() const { return m_quickDelete; }
	IPhysicsCollisionEvent *GetCollisionEventHandler() const { return m_pCollisionEvents; }
	// An object stopped being a trigger: its overlaps end silently.
	void TriggerRemoved( CPhysicsObjectBox3D *pTrigger );
	void WakeTriggerOverlaps( CPhysicsObjectBox3D *pTrigger );
	// Object registration used by object creation from saved/serialized state.
	IPhysicsObject *TrackObject( CPhysicsObjectBox3D *pObject );
	void AddPlayerController( CPlayerControllerBox3D *pController );
	// Drops the contact between two objects (IPhysicsFrictionSnapshot::
	// DeleteAllMarkedContacts).
	void DeleteContactPair( CPhysicsObjectBox3D *pA, CPhysicsObjectBox3D *pB, bool wake );
	// Re-runs the collision rules on the object's existing and nearby pairs
	// (IVP's recheck_collision_filter).
	void RecheckPairs( CPhysicsObjectBox3D *pObject );
	void RemovePlayerController( CPlayerControllerBox3D *pController );
	CPlayerControllerBox3D *FindPlayerController( IPhysicsObject *pObject ) const;
	// Linked providers (restore): fluids, springs and constraints that a
	// restored object's references resolve to.
	CConstraintBox3D *TrackConstraint( IPhysicsObject *pReference, IPhysicsObject *pAttached,
		IPhysicsConstraintGroup *pGroup, int type, const constraint_breakableparams_t &breakable );

private:
	static bool PreSolve( b3ShapeId shapeIdA, b3ShapeId shapeIdB, b3Pos point, b3Vec3 normal, void *pContext );
	bool PairAllowed( CPhysicsObjectBox3D *pA, CPhysicsObjectBox3D *pB ) const;
	void UpdateDeletedPairs();
	static bool CustomFilter( b3ShapeId shapeIdA, b3ShapeId shapeIdB, void *pContext );
	void Step( float dt );
	void PreStep( float dt );
	void PostStep( float dt );
	void CheckConstraintBreaks( float dt );
	void DispatchContactEvents( float dt );
	void DispatchTouchEvents();
	void DispatchSensorEvents();
	void DispatchFrictionEvents( float dt );
	void DispatchSleepWakeEvents();
	void ClearDeadObjects();
	// Unlinks everything that references an object about to go away (or
	// leave this environment).
	void DetachObject( CPhysicsObjectBox3D *pObject, bool notifyConstraints );
	bool IsLive( const IPhysicsObject *pObject ) const;
	float PairDeltaTime( IPhysicsObject *pA, IPhysicsObject *pB );
	bool SaveVehicle( const physsaveparams_t &params );
	bool RestoreVehicle( const physrestoreparams_t &params );

	struct TriggerOverlap_t
	{
		CPhysicsObjectBox3D *pTrigger;
		CPhysicsObjectBox3D *pObject;
		int shapeCount;
	};
	struct ImpactPair_t
	{
		IPhysicsObject *pA;
		IPhysicsObject *pB;
		float lastTime;
	};

	b3WorldId m_world;
	int m_workerCount;
	IThreadPool *m_pThreadPool;
	physics_stepprofile_t m_lastProfile;
	// Game solver calls (step profile), counted under m_solverMutex when
	// several workers can make them.
	ThreadId_t m_simulateThread;
	mutable int m_solverCalls;
	mutable int m_solverCallsOffCaller;
	// Accumulated by Step() during one Simulate call.
	double m_stepSeconds;
	double m_preStepSeconds;
	double m_postStepSeconds;
	// With several workers Box3D runs the custom filter, and so the game's
	// collision solver, on worker threads. This serializes those calls so
	// game code is never entered concurrently (RFC 0013).
	mutable std::mutex m_solverMutex;
	Vector m_gravity;
	float m_airDensity;
	float m_timestep;
	float m_timeAccumulator;
	float m_simulationTime;
	int m_stepCount;
	bool m_inSimulation;
	bool m_quickDelete;
	bool m_queueDeleteObject;
	bool m_enableConstraintNotify;
	physics_performanceparams_t m_performance;
	IPhysicsCollisionSolver *m_pSolver;
	IPhysicsCollisionEvent *m_pCollisionEvents;
	IPhysicsObjectEvent *m_pObjectEvents;
	IPhysicsConstraintEvent *m_pConstraintEvents;
	IVPhysicsDebugOverlay *m_pDebugOverlay;
	CUtlVector<IPhysicsObject *> m_objects;
	CUtlVector<IPhysicsObject *> m_deadObjects;
	CUtlVector<CMotionControllerBox3D *> m_motionControllers;
	CUtlVector<CPlayerControllerBox3D *> m_playerControllers;
	CUtlVector<CConstraintBox3D *> m_constraints;
	CUtlVector<CConstraintGroupBox3D *> m_constraintGroups;
	CUtlVector<CPhysicsFluidControllerBox3D *> m_fluids;
	CUtlVector<CPhysicsSpringBox3D *> m_springs;
	CUtlVector<CVehicleControllerBox3D *> m_vehicles;
	// Contacts the game deleted through a friction snapshot (IVP's
	// DeleteAllFrictionPairs), kept off while the collision rules say so.
	struct DeletedPair_t
	{
		CPhysicsObjectBox3D *pA;
		CPhysicsObjectBox3D *pB;
		bool fresh;		// deleted since the last step: off for one step
		bool disabled;
	};
	int FindDeletedPair( const CPhysicsObjectBox3D *pA, const CPhysicsObjectBox3D *pB ) const;
	// Also holds pairs a recheck found the rules now reject (never fresh).
	CUtlVector<DeletedPair_t> m_deletedPairs;
	float m_contactRecycleDistance;		// Box3D's, restored when none are pending
	// Counters since ClearStats (IVP's statistic manager).
	physics_stats_t m_stats;
	CUtlVector<TriggerOverlap_t> m_triggerOverlaps;
	CUtlVector<ImpactPair_t> m_impactPairs;
};

#endif // PHYSICS_ENVIRONMENT_H
