#include "vphysics_interface.h"
#include "vphysics/parallel_step.h"
#include "vphysics/shape_inertia.h"
#include "vphysics/step_profile.h"
#include "vstdlib/jobthread.h"
#include "tier0/dbg.h"
#include "utlmap.h"

#include "box3d/base.h"
#include "box3d/constants.h"
#include "box3d_convert.h"
#include "physics_environment.h"
#include "physics_object.h"
#include "physics_object_hash.h"

// Box3D runs in Source units: its tolerances scale with this length unit.
// It is process-wide and must be set before any hull or world exists, so it
// is set once when the module loads.
//
// Box3D's log reports expected conditions (a convex over the per-hull limits,
// which the collision decoder then splits), so it goes to the developer
// channel rather than stdout.
static void Box3DLog( const char *pMessage )
{
	DevMsg( 2, "Box3D: %s\n", pMessage );
}

static struct CBox3DLengthUnits
{
	CBox3DLengthUnits()
	{
		b3SetLengthUnitsPerMeter( kBox3DInchesPerMeter );
		b3SetLogFcn( Box3DLog );
	}
} s_box3dLengthUnits;

class CPhysicsInterfaceBox3D : public IPhysics {
public:
	virtual ~CPhysicsInterfaceBox3D() {}
	virtual bool Connect( CreateInterfaceFn factory ) { return true; }
	virtual void Disconnect() {}
	virtual InitReturnVal_t Init() { return INIT_OK; }
	virtual void Shutdown() {}
	// The app framework resolves sibling interfaces (collision, surfaceprops)
	// through the primary IPhysics AppSystem's QueryInterface, not the module's
	// exported CreateInterface. Delegate to this module's own factory so those
	// interfaces are reachable, matching stock vphysics (vphysics/main.cpp).
	virtual void *QueryInterface( const char *pInterfaceName )
	{
		CreateInterfaceFn factory = Sys_GetFactoryThis();
		return factory ? factory( pInterfaceName, NULL ) : NULL;
	}
	virtual IPhysicsEnvironment *CreateEnvironment()
	{
		return CreateEnvironmentWithWorkers( 1, NULL );
	}
	virtual void DestroyEnvironment( IPhysicsEnvironment *pEnvironment )
	{
		m_environments.FindAndRemove( pEnvironment );
		delete pEnvironment;
	}
	virtual IPhysicsEnvironment *GetActiveEnvironmentByIndex( int index )
	{
		return m_environments.IsValidIndex( index ) ? m_environments[index] : NULL;
	}
	virtual int GetActiveEnvironmentCount() { return m_environments.Count(); }
	virtual IPhysicsObjectPairHash *CreateObjectPairHash() { return new CObjectPairHashBox3D(); }
	virtual void DestroyObjectPairHash( IPhysicsObjectPairHash *pHash ) { delete pHash; }
	virtual IPhysicsCollisionSet *FindOrCreateCollisionSet( unsigned int id, int maxElementCount )
	{
		int index = m_collisionSets.Find( id );
		if ( index != m_collisionSets.InvalidIndex() )
			return m_collisionSets[index];
		CCollisionSetBox3D *pSet = new CCollisionSetBox3D();
		m_collisionSets.Insert( id, pSet );
		return pSet;
	}
	virtual IPhysicsCollisionSet *FindCollisionSet( unsigned int id )
	{
		int index = m_collisionSets.Find( id );
		return index != m_collisionSets.InvalidIndex() ? m_collisionSets[index] : NULL;
	}
	virtual void DestroyAllCollisionSets()
	{
		for ( int i = m_collisionSets.FirstInorder(); i != m_collisionSets.InvalidIndex(); i = m_collisionSets.NextInorder( i ) )
			delete m_collisionSets[i];
		m_collisionSets.RemoveAll();
	}

	// Provider internals shared with the parallel-step capability.
	IPhysicsEnvironment *CreateEnvironmentWithWorkers( int workerCount, IThreadPool *pThreadPool )
	{
		IPhysicsEnvironment *pEnvironment =
		    new CPhysicsEnvironmentBox3D( workerCount, pThreadPool );
		m_environments.AddToTail( pEnvironment );
		return pEnvironment;
	}
	// The environment of an object this provider created, or NULL.
	const CPhysicsEnvironmentBox3D *FindObjectEnvironment( const IPhysicsObject *pObject ) const
	{
		for ( int i = 0; pObject && i < m_environments.Count(); i++ )
		{
			const CPhysicsEnvironmentBox3D *pEnvironment =
			    static_cast<const CPhysicsEnvironmentBox3D *>( m_environments[i] );
			if ( pEnvironment->ContainsObject( pObject ) )
				return pEnvironment;
		}
		return NULL;
	}
	bool OwnsEnvironment( const IPhysicsEnvironment *pEnvironment ) const
	{
		for ( int i = 0; i < m_environments.Count(); i++ )
		{
			if ( m_environments[i] == pEnvironment )
				return true;
		}
		return false;
	}

private:
	CUtlMap<unsigned int, CCollisionSetBox3D *> m_collisionSets{ DefLessFunc( unsigned int ) };
	CUtlVector<IPhysicsEnvironment *> m_environments;
};

static CPhysicsInterfaceBox3D g_MainDLLInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsInterfaceBox3D, IPhysics, VPHYSICS_INTERFACE_VERSION, g_MainDLLInterface );

// RFC 0013 vphysics.parallel-step.v1: environments whose Box3D worker tasks
// run on the caller's thread pool. The worker count is fixed at creation
// because Box3D sizes its per-worker state then, and it may not exceed the
// pool's threads plus the calling thread, so the pool is the worker budget.
// Box3D's step is deterministic for any worker count (upstream docs/faq.md);
// the shared benchmark gate checks that claim through the public object state
// (tools/quality/physics_bench.py).
class CPhysicsParallelStepBox3D : public IPhysicsParallelStep
{
public:
	virtual int GetMaxWorkerCount() const
	{
		int logical = GetCPUInformation()->m_nLogicalProcessors;
		return clamp( logical, 1, B3_MAX_WORKERS );
	}
	virtual IPhysicsEnvironment *CreateParallelEnvironment( const physics_parallelparams_t &params )
	{
		if ( params.workerCount < 1 || params.workerCount > GetMaxWorkerCount() )
			return NULL;
		if ( params.workerCount > 1 &&
		     ( !params.pThreadPool || params.workerCount > params.pThreadPool->NumThreads() + 1 ) )
			return NULL;
		return g_MainDLLInterface.CreateEnvironmentWithWorkers(
		    params.workerCount, params.pThreadPool );
	}
	virtual int GetWorkerCount( const IPhysicsEnvironment *pEnvironment ) const
	{
		if ( !pEnvironment || !g_MainDLLInterface.OwnsEnvironment( pEnvironment ) )
			return 0;
		return static_cast<const CPhysicsEnvironmentBox3D *>( pEnvironment )->GetWorkerCount();
	}
	virtual bool IsWorkerCountInvariant() const { return true; }
};

static CPhysicsParallelStepBox3D g_ParallelStep;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsParallelStepBox3D, IPhysicsParallelStep,
    VPHYSICS_PARALLEL_STEP_INTERFACE_VERSION, g_ParallelStep );

// RFC 0013 vphysics.step-profile.v1: solver time against the provider's own
// serial work in the last Simulate call.
class CPhysicsStepProfileBox3D : public IPhysicsStepProfile
{
public:
	virtual bool GetLastSimulate(
	    const IPhysicsEnvironment *pEnvironment, physics_stepprofile_t *pProfile ) const
	{
		if ( !pEnvironment || !pProfile || !g_MainDLLInterface.OwnsEnvironment( pEnvironment ) )
			return false;
		*pProfile =
		    static_cast<const CPhysicsEnvironmentBox3D *>( pEnvironment )->GetLastSimulateProfile();
		return true;
	}
};

static CPhysicsStepProfileBox3D g_StepProfile;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsStepProfileBox3D, IPhysicsStepProfile,
    VPHYSICS_STEP_PROFILE_INTERFACE_VERSION, g_StepProfile );

// RFC 0013 vphysics.shape-inertia.v1: an environment's objects can take the
// full inertia tensor of their collision solid (Box3D's hull mass data about
// Source's mass center) instead of IVP's per-axis inertia; see
// CPhysicsObjectBox3D::ComputeShapeInertia.
class CPhysicsShapeInertiaBox3D : public IPhysicsShapeInertia
{
public:
	virtual bool SetInertiaModel( IPhysicsEnvironment *pEnvironment, physics_inertia_model_t model )
	{
		if ( !pEnvironment || !g_MainDLLInterface.OwnsEnvironment( pEnvironment ) )
			return false;
		return static_cast<CPhysicsEnvironmentBox3D *>( pEnvironment )->SetInertiaModel( model );
	}
	virtual physics_inertia_model_t GetInertiaModel( const IPhysicsEnvironment *pEnvironment ) const
	{
		if ( !pEnvironment || !g_MainDLLInterface.OwnsEnvironment( pEnvironment ) )
			return PHYSICS_INERTIA_LEGACY;
		return static_cast<const CPhysicsEnvironmentBox3D *>( pEnvironment )->GetInertiaModel();
	}
	virtual bool GetInertiaTensor( const IPhysicsObject *pObject, float tensor[3][3] ) const
	{
		if ( !g_MainDLLInterface.FindObjectEnvironment( pObject ) )
			return false;
		static_cast<const CPhysicsObjectBox3D *>( pObject )->GetInertiaTensor( tensor );
		return true;
	}
};

static CPhysicsShapeInertiaBox3D g_ShapeInertia;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsShapeInertiaBox3D, IPhysicsShapeInertia,
    VPHYSICS_SHAPE_INERTIA_INTERFACE_VERSION, g_ShapeInertia );

#include "physics_collision.h"
static CPhysicsCollisionBox3D g_PhysicsCollision;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsCollisionBox3D, IPhysicsCollision, VPHYSICS_COLLISION_INTERFACE_VERSION, g_PhysicsCollision );

#include "physics_material.h"
CPhysicsSurfacePropsBox3D g_SurfaceDatabase;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CPhysicsSurfacePropsBox3D, IPhysicsSurfaceProps, VPHYSICS_SURFACEPROPS_INTERFACE_VERSION, g_SurfaceDatabase );

DLL_EXPORT IPhysics *Physics_Create() {
	return &g_MainDLLInterface;
}
